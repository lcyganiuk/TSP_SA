#pragma once

#include <string>
#include <vector>

struct TSPInstance {
    std::string name;
    int n = 0;
    bool asymmetric = false;
    std::vector<std::vector<int> > matrix;
};

long long tourCost(const TSPInstance& inst, const std::vector<int>& tour);
