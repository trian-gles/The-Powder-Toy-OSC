#pragma once

#include "simulation/Particle.h"

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <numeric>
#include <cmath>

typedef std::pair<double, double> DistParams;
typedef std::pair<double, double> GaussParams;

typedef struct MasterReturnParams {
    int count;
    DistParams x;
    DistParams y;
    GaussParams vel;
    GaussParams temp;
    bool p_solid;
    bool p_liquid;
    bool p_gas;
    bool p_energy; 
} _MasterReturnParams;


const int MIN = -1000000; // Adjust these constants as needed
const int MAX = 1000000;

class DistributionHandler {
public:
    DistributionHandler();
    void reset();
    void update(int v);
    std::pair<int, int> get() const;

private:
    int min, max;
};

class GaussDistributionHandler {
public:
    void update(float v);
    void reset();
    std::pair<double, double> get() const;
    size_t count() const;

private:
    std::vector<float> samples;
};

class ParticleIdCountSorter {
public:
    ParticleIdCountSorter();
    void update(int v);
    std::vector<int> getres();
    void reset();

private:
    std::unordered_map<int, int> idCounts;
    std::vector<int> types;
};

class MasterHandler {
public:
    MasterHandler();
    ~MasterHandler();
    void update(Particle* p);
    void get(MasterReturnParams* params) const;
    void reset();

    int p_type;

private:
    GaussDistributionHandler* yHandler;
    DistributionHandler* xHandler;
    GaussDistributionHandler* velHandler;
    GaussDistributionHandler* tempHandler;
    int p_index;
    int p_count;
    
    bool p_solid;
    bool p_liquid;
    bool p_gas;
    bool p_energy;

    int getSimPartProperty(int index, const std::string& property) const;
    int getElementProperty(int index, const std::string& property) const;
    int bitAnd(int props, const std::string& type) const;
    double scale(double min, double max, double value) const;
};

