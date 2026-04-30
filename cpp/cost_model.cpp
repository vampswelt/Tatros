// cost_model.cpp
#include "cost_model.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
using namespace std;

void CostModel::applyToGraph(Graph& /*g*/) const {

double DistanceCost::compute(const Edge& e) const {
    return e.distance_m;   
}


double TimeCost::compute(const Edge& e) const {
    double speed_kmh = (e.max_speed_kmh > 0) ? e.max_speed_kmh : 40.0;
    double speed_ms  = speed_kmh / 3.6;
    return e.distance_m / speed_ms;   // seconds
}

-
double HybridCost::compute(const Edge& e) const {
    constexpr double REF_DIST = 1000.0;  
    constexpr double REF_TIME = 60.0;    

    double speed_kmh = (e.max_speed_kmh > 0) ? e.max_speed_kmh : 40.0;
    double speed_ms  = speed_kmh / 3.6;
    double time_s    = e.distance_m / speed_ms;

    double norm_dist = e.distance_m / REF_DIST;
    double norm_time = time_s       / REF_TIME;

    
    return (alpha_ * norm_time + (1.0 - alpha_) * norm_dist) * REF_TIME;
}

CostModel* makeCostModel(const std::string& mode, double alpha) {
    if (mode == "distance") return new DistanceCost();
    if (mode == "time")     return new TimeCost();
    if (mode == "hybrid")   return new HybridCost(alpha);
    cerr << "[CostModel] Unknown mode '" << mode << "', defaulting to 'distance'.\n";
    return new DistanceCost();
}
