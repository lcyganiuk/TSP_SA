#include "sa.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>

using namespace std;

long long tourCost(const TSPInstance& inst, const vector<int>& tour) {
    long long cost = 0;
    for (int i = 0; i + 1 < (int)tour.size(); ++i) cost += inst.matrix[tour[i]][tour[i + 1]];
    cost += inst.matrix[tour.back()][tour.front()];
    return cost;
}

static vector<int> nearestFrom(const TSPInstance& inst, int start, long long& cost) {
    int n = inst.n;
    vector<int> tour;
    vector<char> used(n, 0);
    tour.reserve(n);
    int current = start;
    used[current] = 1;
    tour.push_back(current);
    cost = 0;

    for (int step = 1; step < n; ++step) {
        int best = -1;
        int bestWeight = numeric_limits<int>::max();
        for (int v = 0; v < n; ++v) {
            if (!used[v] && inst.matrix[current][v] < bestWeight) {
                bestWeight = inst.matrix[current][v];
                best = v;
            }
        }
        used[best] = 1;
        tour.push_back(best);
        cost += bestWeight;
        current = best;
    }
    cost += inst.matrix[current][start];
    return tour;
}

static vector<int> multiStartNN(const TSPInstance& inst, long long& bestCost) {
    bestCost = numeric_limits<long long>::max();
    vector<int> bestTour;
    for (int s = 0; s < inst.n; ++s) {
        long long c = 0;
        vector<int> t = nearestFrom(inst, s, c);
        if (c < bestCost) {
            bestCost = c;
            bestTour = t;
        }
    }
    return bestTour;
}

static long long mstLowerBound(const TSPInstance& inst) {
    int n = inst.n;
    vector<int> minEdge(n, numeric_limits<int>::max());
    vector<char> inTree(n, 0);
    minEdge[0] = 0;
    long long total = 0;

    for (int it = 0; it < n; ++it) {
        int v = -1;
        for (int i = 0; i < n; ++i) {
            if (!inTree[i] && (v == -1 || minEdge[i] < minEdge[v])) v = i;
        }
        inTree[v] = 1;
        total += minEdge[v];
        for (int to = 0; to < n; ++to) {
            if (to == v || inTree[to]) continue;
            int w = inst.asymmetric ? min(inst.matrix[v][to], inst.matrix[to][v]) : inst.matrix[v][to];
            if (w < minEdge[to]) minEdge[to] = w;
        }
    }
    return total;
}

static void applyMove(vector<int>& tour, int i, int j, const string& type) {
    if (type == "swap") {
        swap(tour[i], tour[j]);
    } else if (type == "insert") {
        int city = tour[j];
        tour.erase(tour.begin() + j);
        if (j < i) --i;
        tour.insert(tour.begin() + i, city);
    } else {
        if (i > j) swap(i, j);
        reverse(tour.begin() + i, tour.begin() + j + 1);
    }
}

static vector<int> randomTour(int n, mt19937& rng) {
    vector<int> tour(n);
    iota(tour.begin(), tour.end(), 0);
    shuffle(tour.begin() + 1, tour.end(), rng);
    return tour;
}

static double estimateInitialTemperature(const TSPInstance& inst,
                                         const vector<int>& baseTour,
                                         const Config& cfg,
                                         mt19937& rng) {
    uniform_int_distribution<int> dist(1, inst.n - 1);
    vector<int> tour = baseTour;
    long long current = tourCost(inst, tour);
    double positiveDeltaSum = 0.0;
    int positiveCount = 0;
    int samples = max(1, cfg.initialTemperatureSamples);

    for (int s = 0; s < samples; ++s) {
        vector<int> next = tour;
        int i = dist(rng);
        int j = dist(rng);
        if (i == j) continue;
        applyMove(next, i, j, cfg.neighbourhood);
        long long nc = tourCost(inst, next);
        long long delta = nc - current;
        if (delta > 0) {
            positiveDeltaSum += (double)delta;
            ++positiveCount;
        }
    }

    double avgDelta = positiveCount > 0 ? positiveDeltaSum / positiveCount : 1.0;
    double acceptance = min(0.99, max(0.01, cfg.initialAcceptance));
    double t0 = -avgDelta / log(acceptance);
    t0 *= max(0.0001, cfg.initialTemperatureMultiplier);
    return max(0.0001, t0);
}

static double relativeError(long long value, long long reference) {
    if (reference <= 0) return -1.0;
    return 100.0 * ((double)value - (double)reference) / (double)reference;
}

SAResult simulatedAnnealing(const TSPInstance& inst, const Config& cfg, const InstanceConfig& icfg) {
    using clock = chrono::steady_clock;
    mt19937 rng(cfg.seed);
    uniform_real_distribution<double> prob(0.0, 1.0);
    uniform_int_distribution<int> cityDist(1, inst.n - 1);

    SAResult res;
    res.lowerBound = cfg.useLB ? mstLowerBound(inst) : 0;

    vector<int> currentTour;
    if (cfg.useUB) currentTour = multiStartNN(inst, res.initialCost);
    else {
        currentTour = randomTour(inst.n, rng);
        res.initialCost = tourCost(inst, currentTour);
    }

    long long currentCost = res.initialCost;
    res.bestCost = currentCost;
    res.bestTour = currentTour;
    res.initialTemperature = estimateInitialTemperature(inst, currentTour, cfg, rng);
    double temperature = res.initialTemperature;
    int epochLength = cfg.epochLength > 0 ? cfg.epochLength : max(1, (int)(cfg.epochFactor * inst.n));

    ofstream trace;
    if (!cfg.logFile.empty()) {
        bool append = false;
        ifstream existing(cfg.logFile.c_str());
        append = existing.good();
        existing.close();
        trace.open(cfg.logFile.c_str(), ios::app);
        if (!append) trace << "instance;n;time_ms;iteration;temperature;current_cost;best_cost;lb;opt\n";
    }

    auto start = clock::now();
    auto lastLog = start;
    int noImproveEpochs = 0;

    while (true) {
        bool improvedInEpoch = false;
        for (int it = 0; it < epochLength; ++it) {
            int i = cityDist(rng);
            int j = cityDist(rng);
            if (i == j) continue;

            vector<int> nextTour = currentTour;
            applyMove(nextTour, i, j, cfg.neighbourhood);
            long long nextCost = tourCost(inst, nextTour);
            long long delta = nextCost - currentCost;

            if (delta <= 0 || prob(rng) < exp(-(double)delta / temperature)) {
                currentTour.swap(nextTour);
                currentCost = nextCost;
            }

            if (currentCost < res.bestCost) {
                res.bestCost = currentCost;
                res.bestTour = currentTour;
                improvedInEpoch = true;
                noImproveEpochs = 0;
            }

            ++res.iterations;
            auto now = clock::now();
            double elapsedMs = chrono::duration<double, milli>(now - start).count();
            if (cfg.logIntervalMs > 0 && chrono::duration_cast<chrono::milliseconds>(now - lastLog).count() >= cfg.logIntervalMs) {
                if (trace.is_open()) {
                    trace << inst.name << ';' << inst.n << ';' << elapsedMs << ';' << res.iterations << ';'
                          << temperature << ';' << currentCost << ';' << res.bestCost << ';'
                          << res.lowerBound << ';' << icfg.optimum << '\n';
                }
                lastLog = now;
            }

            bool targetByOpt = icfg.optimum > 0 && relativeError(res.bestCost, icfg.optimum) <= cfg.targetErrorPercent;
            bool targetByLb = cfg.useLB && icfg.optimum <= 0 && res.lowerBound > 0 && relativeError(res.bestCost, res.lowerBound) <= cfg.targetErrorPercent;
            if ((targetByOpt || targetByLb) && elapsedMs >= cfg.minTimeBeforeTargetMs) {
                res.targetReached = true;
                res.stopReason = targetByOpt ? "osiagnieto wymagany blad wzgledem OPT"
                                             : "osiagnieto wymagany prog wzgledem LB";
                goto finish;
            }
            if (elapsedMs >= cfg.timeLimitMs) {
                res.stopReason = "limit czasu";
                goto finish;
            }
        }

        ++res.epochs;
        if (!improvedInEpoch) ++noImproveEpochs;
        if (cfg.cooling == "logarithmic") temperature = res.initialTemperature / log((double)res.epochs + 2.0);
        else temperature *= cfg.lambda;

        if (temperature < 0.000001) temperature = 0.000001;
        if (noImproveEpochs >= cfg.maxNoImproveEpochs) {
            res.stopReason = "brak poprawy przez max_no_improve_epochs";
            break;
        }
    }

finish:
    if (res.stopReason == "nieznany") res.stopReason = "zakonczono";
    res.finalTemperature = temperature;
    res.noImproveEpochs = noImproveEpochs;
    res.timeMs = chrono::duration<double, milli>(clock::now() - start).count();
    res.bestErrorPercent = relativeError(res.bestCost, icfg.optimum);
    res.bestGapToLBPercent = relativeError(res.bestCost, res.lowerBound);
    if (trace.is_open()) {
        trace << inst.name << ';' << inst.n << ';' << res.timeMs << ';' << res.iterations << ';'
              << temperature << ';' << currentCost << ';' << res.bestCost << ';'
              << res.lowerBound << ';' << icfg.optimum << '\n';
    }
    return res;
}
