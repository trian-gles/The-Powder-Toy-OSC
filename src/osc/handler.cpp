#include "handler.h"
#include "simulation/Particle.h"
#include "simulation/Element.h"
#include "simulation/ElementClasses.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <tuple>
#include <utility>

// Constructor for BinHandler
BinHandler::BinHandler(int count) : binCount(count) {
    bins.resize(binCount, 0);
}

// Reset all bins to zero
void BinHandler::reset() {
    for (int i = 0; i < binCount; ++i) {
        bins[i] = 0;
    }
}

// Update a specific bin by incrementing its count
void BinHandler::update(int index) {
    if (index >= 1 && index <= binCount) {
        bins[index - 1]++;
    }
}

// Get the current bin counts
std::vector<int> BinHandler::get() {
    return bins;
}

// Constructor for PlantHandler
PlantHandler::PlantHandler() 
    : newBins(16), deletedBins(16) {}

// Reset all plant states
void PlantHandler::reset() {
    newBins.reset();
    deletedBins.reset();
}

// Update plant state at the given index
void PlantHandler::update(int y) {
    int bin = static_cast<int>(std::floor(16 * (383 - y) / 383.0)) + 1;
    newBins.update(bin);
}

void PlantHandler::kill(int y){
    int bin = static_cast<int>(std::floor(16 * (383 - y) / 383.0)) + 1;
    deletedBins.update(bin);
}


// Get the bin data for new, old, and deleted plants
std::tuple<std::vector<int>, std::vector<int>> PlantHandler::get() {

    return {newBins.get(), deletedBins.get()};
}



DistributionHandler::DistributionHandler() : min(MIN), max(MAX) {}

void DistributionHandler::reset() {
    min = MIN;
    max = MAX;
}

void DistributionHandler::update(int v) {
    max = std::max(max, v);
    min = std::min(min, v);
}

std::pair<int, int> DistributionHandler::get() const {
    return {min, max};
}

void GaussDistributionHandler::update(float v) {
    samples.push_back(v);
}

void GaussDistributionHandler::reset() {
    samples.clear();
}

std::pair<double, double> GaussDistributionHandler::get() const {
    if (samples.empty()) {
        return {0.0, 0.0};
    }

    double mu = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();

    double sigma = 0.0;
    for (float v : samples) {
        sigma += (v - mu) * (v - mu);
    }
    sigma = std::sqrt(sigma / samples.size());

    return {mu, sigma};
}

size_t GaussDistributionHandler::count() const {
    return samples.size();
}

ParticleIdCountSorter::ParticleIdCountSorter(): idCounts(), types() {
}


void ParticleIdCountSorter::update(int v) {
    idCounts[v]++;
    if (std::find(types.begin(), types.end(), v) == types.end()) {
        types.push_back(v);
    }
}

std::vector<int> ParticleIdCountSorter::getres() {
    std::sort(types.begin(), types.end(), [&](int k1, int k2) {
        return idCounts[k1] > idCounts[k2];
    });
    return types;
}

void ParticleIdCountSorter::reset() {
    idCounts.clear();
    types.clear();
}



MasterHandler::MasterHandler() 
    : yHandler(new GaussDistributionHandler()), 
      xHandler(new DistributionHandler()), 
      velHandler(new GaussDistributionHandler()), 
      tempHandler(new GaussDistributionHandler()), 
      p_index(0), p_count(0), p_type(0), p_solid(0), p_liquid(0), p_gas(0), p_energy(0) {}

MasterHandler::~MasterHandler() {
    delete yHandler;
    delete xHandler;
    delete velHandler;
    delete tempHandler;
}

void MasterHandler::update(Particle* p) {
    
    yHandler->update(p->y);
    xHandler->update(p->x);
    velHandler->update(static_cast<float>(std::sqrt(p->vx * p->vx + p->vy * p->vy)));
    tempHandler->update(p->temp);
    p_count++;
    if (p_count == 1){
        this->p_type = p->type;
        auto elementClasses = GetElements();
        auto element = &elementClasses[p->type];
        int props = element->Properties;
        p_solid = props & TYPE_PART;
        p_liquid = props & TYPE_LIQUID;
        p_gas = props & TYPE_GAS;
        p_energy = props & TYPE_ENERGY;
    }
    

}

void MasterHandler::get(MasterReturnParams* params) const {
    
    auto [muY, sigY] = yHandler->get();
    auto [minX, maxX] = xHandler->get();
    auto [muVel, sigVel] = velHandler->get();
    auto [muTemp, sigTemp] = tempHandler->get();
    muTemp = scale(0, 2100, muTemp);

    params->count = p_count;
    params->y.first = muY;
    params->y.second = sigY;
    params->x.first = minX;
    params->x.second = maxX;
    params->vel.first = muVel;
    params->vel.second = sigVel;
    params->temp.first = muTemp;
    params->temp.second = sigTemp;
    params->p_solid = p_solid;
    params->p_liquid = p_liquid;
    params->p_gas = p_gas;
    params->p_energy = p_energy;
    

    /*
    int props = getElementProperty(p_index, "Properties");
    int liquid = std::min(bitAnd(props, "TYPE_LIQUID"), 1);
    int powder = std::min(bitAnd(props, "TYPE_PART"), 1);
    int gas = std::min(bitAnd(props, "TYPE_GAS"), 1);
    int energy = std::min(bitAnd(props, "TYPE_ENERGY"), 1);
    */

    // Implement the required output handling
}

void MasterHandler::reset() {
    yHandler->reset();
    xHandler->reset();
    velHandler->reset();
    tempHandler->reset();
    p_count = 0;
}


double MasterHandler::scale(double min, double max, double value) const {
    return (value - min) / (max - min);
}
