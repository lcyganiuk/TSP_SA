#pragma once

#include "config.h"
#include "tsp.h"
#include <vector>

struct SAResult {
    long long initialCost = 0;
    long long lowerBound = 0;
    long long bestCost = 0;
    double bestErrorPercent = -1.0;
    double bestGapToLBPercent = -1.0;
    long long iterations = 0;
    int epochs = 0;
    int noImproveEpochs = 0;
    double initialTemperature = 0.0;
    double finalTemperature = 0.0;
    double timeMs = 0.0;
    bool targetReached = false;
    std::string stopReason = "nieznany";
    std::vector<int> bestTour;
};

SAResult simulatedAnnealing(const TSPInstance& inst,
                            const Config& cfg,
                            const InstanceConfig& icfg);
