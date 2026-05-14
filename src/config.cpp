#include "config.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

static string trim(string s) {
    size_t comment = s.find('#');
    if (comment != string::npos) s = s.substr(0, comment);
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t p = 0;
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) ++p;
    return s.substr(p);
}

static string lower(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)tolower(c); });
    return s;
}

static bool parseBool(const string& value) {
    string v = lower(value);
    return v == "1" || v == "true" || v == "tak" || v == "yes";
}

static bool isAbsolutePath(const string& path) {
    return path.size() >= 3 && isalpha((unsigned char)path[0]) && path[1] == ':' &&
           (path[2] == '\\' || path[2] == '/');
}

static string directoryOf(const string& path) {
    size_t pos = path.find_last_of("\\/");
    if (pos == string::npos) return ".";
    return path.substr(0, pos);
}

static string joinPath(const string& dir, const string& path) {
    if (path.empty() || isAbsolutePath(path) || dir == ".") return path;
    char last = dir[dir.size() - 1];
    if (last == '\\' || last == '/') return dir + path;
    return dir + "\\" + path;
}

static void setOption(Config& cfg, const string& key, const string& value) {
    string k = lower(key);
    if (k == "time_limit_ms") cfg.timeLimitMs = atoi(value.c_str());
    else if (k == "min_time_before_target_ms") cfg.minTimeBeforeTargetMs = atoi(value.c_str());
    else if (k == "max_no_improve_epochs") cfg.maxNoImproveEpochs = atoi(value.c_str());
    else if (k == "seed") cfg.seed = (unsigned int)strtoul(value.c_str(), nullptr, 10);
    else if (k == "use_ub") cfg.useUB = parseBool(value);
    else if (k == "use_lb") cfg.useLB = parseBool(value);
    else if (k == "cooling") cfg.cooling = lower(value);
    else if (k == "neighbourhood") cfg.neighbourhood = lower(value);
    else if (k == "lambda") cfg.lambda = atof(value.c_str());
    else if (k == "initial_acceptance") cfg.initialAcceptance = atof(value.c_str());
    else if (k == "initial_temperature_multiplier") cfg.initialTemperatureMultiplier = atof(value.c_str());
    else if (k == "initial_temperature_samples") cfg.initialTemperatureSamples = atoi(value.c_str());
    else if (k == "epoch_length") cfg.epochLength = atoi(value.c_str());
    else if (k == "epoch_factor") cfg.epochFactor = atof(value.c_str());
    else if (k == "target_error_percent") cfg.targetErrorPercent = atof(value.c_str());
    else if (k == "log_interval_ms") cfg.logIntervalMs = atoi(value.c_str());
    else if (k == "log_file") cfg.logFile = value;
    else cout << "UWAGA: Pominieto nieznany parametr config: " << key << endl;
}

Config loadConfig(const string& filename) {
    string usedFilename = filename;
    ifstream file(usedFilename.c_str());
    if (!file.is_open() && filename == "config.txt") {
        usedFilename = "..\\config.txt";
        file.open(usedFilename.c_str());
    }

    if (!file.is_open()) {
        cout << "BLAD: Nie mozna otworzyc pliku config: " << filename << endl;
        cout << "Uruchom program z katalogu zad3 albo zad3\\cmake-build-debug." << endl;
        exit(1);
    }

    Config cfg;
    cfg.baseDir = directoryOf(usedFilename);
    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) continue;

        stringstream ss(line);
        string first;
        ss >> first;
        if (lower(first) == "instance") {
            InstanceConfig icfg;
            int asym = 0;
            ss >> icfg.file >> icfg.optimum >> asym;
            icfg.file = joinPath(cfg.baseDir, icfg.file);
            icfg.asymmetric = asym != 0;
            if (icfg.file.empty()) {
                cout << "BLAD: Linia instance wymaga: instance <plik> <OPT albo 0> <ATSP: 0/1>" << endl;
                exit(1);
            }
            cfg.instances.push_back(icfg);
        } else {
            string value;
            ss >> value;
            if (value.empty()) {
                cout << "BLAD: Parametr config bez wartosci: " << first << endl;
                exit(1);
            }
            setOption(cfg, first, value);
        }
    }

    cfg.logFile = joinPath(cfg.baseDir, cfg.logFile);

    if (cfg.instances.empty()) {
        cout << "BLAD: Brak instancji. Dodaj linie: instance <plik> <OPT albo 0> <ATSP: 0/1>" << endl;
        exit(1);
    }

    if (cfg.lambda <= 0.0 || cfg.lambda >= 1.0) {
        cout << "BLAD: lambda musi byc w zakresie (0,1), w badaniu zwykle 0.85..0.999" << endl;
        exit(1);
    }

    return cfg;
}
