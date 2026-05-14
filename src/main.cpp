#include "config.h"
#include "loader.h"
#include "sa.h"

#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

static void printTour(const vector<int>& tour) {
    cout << "tour=";
    for (int city : tour) cout << city << ' ';
    cout << endl;
}

static void printConfigSummary(const Config& cfg) {
    cout << "\nParametry uruchomienia:\n";
    cout << "  limit czasu [ms]: " << cfg.timeLimitMs << '\n';
    cout << "  UB: " << (cfg.useUB ? "tak" : "nie") << ", LB: " << (cfg.useLB ? "tak" : "nie") << '\n';
    cout << "  chlodzenie: " << cfg.cooling << ", lambda: " << cfg.lambda << '\n';
    cout << "  sasiedztwo: " << cfg.neighbourhood << '\n';
    cout << "  target_error_percent: " << cfg.targetErrorPercent << '\n';
    cout << "  min_time_before_target_ms: " << cfg.minTimeBeforeTargetMs << '\n';
    cout << "  max_no_improve_epochs: " << cfg.maxNoImproveEpochs << '\n';
    cout << "  initial_acceptance: " << cfg.initialAcceptance << '\n';
    cout << "  initial_temperature_multiplier: " << cfg.initialTemperatureMultiplier << '\n';
    cout << "  epoch_length: " << cfg.epochLength
         << (cfg.epochLength == 0 ? " (auto = epoch_factor * n)" : "") << '\n';
    cout << "  epoch_factor: " << cfg.epochFactor << '\n';
    cout << "  log_file: " << cfg.logFile << '\n';
}

int main() {
    Config cfg = loadConfig("config.txt");

    printConfigSummary(cfg);
    cout << fixed << setprecision(3);

    for (const auto& icfg : cfg.instances) {
        TSPInstance inst = loadInstance(icfg.file);
        inst.asymmetric = icfg.asymmetric || inst.asymmetric;

        cout << "\n============================================================" << endl;
        cout << "Instancja: " << inst.name << endl;
        cout << "n: " << inst.n << ", typ: " << (inst.asymmetric ? "ATSP" : "TSP") << endl;
        cout << "OPT: " << icfg.optimum
             << (icfg.optimum > 0 ? "" : " (brak - blad wzgledem OPT nie bedzie liczony)") << endl;

        SAResult res = simulatedAnnealing(inst, cfg, icfg);

        cout << "LB: " << res.lowerBound << endl;
        cout << "Koszt startowy" << (cfg.useUB ? " / UB" : " losowy") << ": " << res.initialCost << endl;
        cout << "Najlepszy koszt SA: " << res.bestCost << endl;
        if (icfg.optimum > 0) {
            cout << "Blad wzgledem OPT [%]: " << res.bestErrorPercent << endl;
            cout << "Prog wymagany [%]: " << cfg.targetErrorPercent << endl;
            cout << "Wymagany prog osiagniety: " << (res.targetReached ? "TAK" : "NIE") << endl;
        } else {
            cout << "Blad wzgledem OPT [%]: brak OPT w configu" << endl;
            cout << "Luka wzgledem LB [%]: " << res.bestGapToLBPercent << endl;
        }
        cout << "Powod zatrzymania: " << res.stopReason << endl;
        cout << "Czas [ms]: " << res.timeMs
             << ", iteracje: " << res.iterations
             << ", epoki: " << res.epochs << endl;
        cout << "T0: " << res.initialTemperature
             << ", T_koncowe: " << res.finalTemperature << endl;


        if (inst.n <= 50) printTour(res.bestTour);
        system("pause");
    }

    return 0;
}
