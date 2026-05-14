#pragma once

#include <string>
#include <vector>

struct InstanceConfig {
    std::string file;
    long long optimum = 0;
    bool asymmetric = false;
};

struct Config {
    std::vector<InstanceConfig> instances;
    std::string baseDir = ".";
    int timeLimitMs = 900000;
    int minTimeBeforeTargetMs = 0;
    int maxNoImproveEpochs = 200;
    unsigned int seed = 1;
    bool useUB = true;
    bool useLB = true;
    std::string cooling = "geometric";
    std::string neighbourhood = "inverse";
    double lambda = 0.995;
    double initialAcceptance = 0.80;
    double initialTemperatureMultiplier = 1.0;
    int initialTemperatureSamples = 2000;
    int epochLength = 0;
    double epochFactor = 20.0;
    double targetErrorPercent = 10.0;
    int logIntervalMs = 1000;
    std::string logFile = "sa_trace.csv";
};

Config loadConfig(const std::string& filename);
