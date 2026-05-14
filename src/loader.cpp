#include "loader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

static string trim(string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) s.pop_back();
    size_t p = 0;
    while (p < s.size() && (s[p] == ' ' || s[p] == '\t')) ++p;
    return s.substr(p);
}

static string upper(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return (char)toupper(c); });
    return s;
}

static string valueAfterColon(const string& line) {
    size_t pos = line.find(':');
    if (pos == string::npos) {
        string key, value;
        stringstream ss(line);
        ss >> key >> value;
        return value;
    }
    return trim(line.substr(pos + 1));
}

static int euc2d(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return (int)(sqrt(dx * dx + dy * dy) + 0.5);
}

static int ceil2d(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return (int)ceil(sqrt(dx * dx + dy * dy));
}

static void readSimpleMatrix(ifstream& file, TSPInstance& inst) {
    file.clear();
    file.seekg(0);
    file >> inst.n;
    inst.matrix.assign(inst.n, vector<int>(inst.n, 0));
    for (int i = 0; i < inst.n; ++i) {
        for (int j = 0; j < inst.n; ++j) file >> inst.matrix[i][j];
    }
}

TSPInstance loadInstance(const string& filename) {
    ifstream file(filename.c_str());
    if (!file.is_open()) {
        cout << "BLAD: Nie mozna otworzyc pliku instancji: " << filename << endl;
        exit(1);
    }

    TSPInstance inst;
    inst.name = filename;

    string line;
    int dimension = 0;
    string type, edgeWeightType, edgeWeightFormat;
    streampos sectionPos = 0;
    string section;

    while (getline(file, line)) {
        string t = trim(line);
        string u = upper(t);
        if (u.find("DIMENSION") == 0) dimension = atoi(valueAfterColon(t).c_str());
        else if (u.find("TYPE") == 0) type = upper(valueAfterColon(t));
        else if (u.find("EDGE_WEIGHT_TYPE") == 0) edgeWeightType = upper(valueAfterColon(t));
        else if (u.find("EDGE_WEIGHT_FORMAT") == 0) edgeWeightFormat = upper(valueAfterColon(t));
        else if (u == "NODE_COORD_SECTION" || u == "EDGE_WEIGHT_SECTION") {
            section = u;
            sectionPos = file.tellg();
            break;
        }
    }

    if (dimension <= 0 || section.empty()) {
        readSimpleMatrix(file, inst);
        return inst;
    }

    inst.n = dimension;
    inst.asymmetric = (type == "ATSP");
    inst.matrix.assign(inst.n, vector<int>(inst.n, 0));
    file.clear();
    file.seekg(sectionPos);

    if (section == "NODE_COORD_SECTION") {
        vector<pair<double, double> > coords(inst.n);
        for (int i = 0; i < inst.n; ++i) {
            int id;
            double x, y;
            file >> id >> x >> y;
            if (id >= 1 && id <= inst.n) coords[id - 1] = make_pair(x, y);
        }
        for (int i = 0; i < inst.n; ++i) {
            for (int j = 0; j < inst.n; ++j) {
                if (edgeWeightType == "CEIL_2D") inst.matrix[i][j] = ceil2d(coords[i].first, coords[i].second, coords[j].first, coords[j].second);
                else inst.matrix[i][j] = euc2d(coords[i].first, coords[i].second, coords[j].first, coords[j].second);
            }
        }
        return inst;
    }

    vector<int> values;
    string token;
    while (file >> token) {
        if (upper(token) == "EOF") break;
        values.push_back(atoi(token.c_str()));
    }

    if (edgeWeightFormat.empty() || edgeWeightFormat == "FULL_MATRIX") {
        int p = 0;
        for (int i = 0; i < inst.n; ++i) {
            for (int j = 0; j < inst.n; ++j) {
                if (p < (int)values.size()) inst.matrix[i][j] = values[p++];
            }
        }
    } else if (edgeWeightFormat == "LOWER_DIAG_ROW") {
        int p = 0;
        for (int i = 0; i < inst.n; ++i) {
            for (int j = 0; j <= i; ++j) {
                int w = (p < (int)values.size()) ? values[p++] : 0;
                inst.matrix[i][j] = w;
                inst.matrix[j][i] = w;
            }
        }
    } else if (edgeWeightFormat == "UPPER_ROW") {
        int p = 0;
        for (int i = 0; i < inst.n; ++i) {
            for (int j = i + 1; j < inst.n; ++j) {
                int w = (p < (int)values.size()) ? values[p++] : 0;
                inst.matrix[i][j] = w;
                inst.matrix[j][i] = w;
            }
        }
    } else if (edgeWeightFormat == "UPPER_DIAG_ROW") {
        int p = 0;
        for (int i = 0; i < inst.n; ++i) {
            for (int j = i; j < inst.n; ++j) {
                int w = (p < (int)values.size()) ? values[p++] : 0;
                inst.matrix[i][j] = w;
                inst.matrix[j][i] = w;
            }
        }
    } else {
        cout << "BLAD: Nieobslugiwany EDGE_WEIGHT_FORMAT: " << edgeWeightFormat << endl;
        exit(1);
    }

    return inst;
}
