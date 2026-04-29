#ifndef DIJKSTRA_H
#define DIJKSTRA_H

#include "graph.h"
#include <vector>

struct DijkstraResult {
    vector<long long> path;
    double cost;
    long long explored;
};

DijkstraResult dijkstra(Graph& g, long long start, long long end, double slopeWeight);

#endif