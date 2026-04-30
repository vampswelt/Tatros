#pragma once


#include "graph.h"
#include "cost_model.h"
#include <vector>
#include <limits>
using namespace std;


struct DijkstraResult {
    bool          found          = false;
    double        total_cost     = numeric_limits<double>::infinity();
    double        total_distance = 0.0;   
    double        total_time_s   = 0.0;   
    vector<long long> path;          
};


class Dijkstra {
public:
    explicit Dijkstra(const Graph& graph, const CostModel& model)
        : graph_(graph), model_(model) {}

    
    DijkstraResult run(long long source, long long destination) const;

   
    static void writePathFile(const DijkstraResult& result,
                              const string& out_path);

private:
    const Graph&      graph_;
    const CostModel&  model_;
};
