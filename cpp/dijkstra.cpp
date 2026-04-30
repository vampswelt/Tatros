// dijkstra.cpp
#include "dijkstra.h"

#include <queue>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
using namespace std;

using PQItem = std::pair<double, long long>;

DijkstraResult Dijkstra::run(long long source, long long destination) const
{
    DijkstraResult res;

    if (!graph_.hasNode(source)) {
        cerr << "[Dijkstra] Source node " << source << " not in graph.\n";
        return res;
    }
    if (!graph_.hasNode(destination)) {
        cerr << "[Dijkstra] Destination node " << destination << " not in graph.\n";
        return res;
    }

    unordered_map<long long, double> dist;
    unordered_map<long long, long long> prev;  

    dist[source] = 0.0;

    priority_queue<PQItem, vector<PQItem>, greater<PQItem>> pq;
    pq.push({0.0, source});

    auto t0 = chrono::steady_clock::now();

    size_t relaxations = 0;

    while (!pq.empty()) {
        double d   = pq.top().first;
        long long u = pq.top().second;
        pq.pop();

        if (d > dist[u]) continue;   

        if (u == destination) break; 
        for (const Edge& e : graph_.getNeighbours(u)) {
            double edge_cost = model_.compute(e);
            double new_dist  = dist[u] + edge_cost;

            auto it = dist.find(e.to);
            if (it == dist.end() || new_dist < it->second) {
                dist[e.to] = new_dist;
                prev[e.to] = u;
                pq.push({new_dist, e.to});
                ++relaxations;
            }
        }
    }

    auto t1 = chrono::steady_clock::now();
    double elapsed_ms = chrono::duration<double, milli>(t1 - t0).count();

    // Check reachability
    if (dist.find(destination) == dist.end()) {
        cerr << "[Dijkstra] Destination " << destination << " is unreachable from " << source << ".\n";
        return res;
    }

    vector<long long> path;
    for (long long cur = destination; cur != source; ) {
        path.push_back(cur);
        auto it = prev.find(cur);
        if (it == prev.end()) {
            cerr << "[Dijkstra] Path reconstruction failed at node " << cur << ".\n";
            return res;
        }
        cur = it->second;
    }
    path.push_back(source);
    reverse(path.begin(), path.end());

    double total_dist = 0.0, total_time = 0.0;
    TimeCost tc;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        long long u = path[i], v = path[i + 1];
        for (const Edge& e : graph_.getNeighbours(u)) {
            if (e.to == v) {
                total_dist += e.distance_m;
                total_time += tc.compute(e);
                break;
            }
        }
    }

    res.found          = true;
    res.total_cost     = dist[destination];
    res.total_distance = total_dist;
    res.total_time_s   = total_time;
    res.path           = std::move(path);

    cout << "[Dijkstra] Path found in " << elapsed_ms << " ms"
              << " | relaxations=" << relaxations
              << " | nodes visited=" << dist.size() << "\n";

    return res;
}


void Dijkstra::writePathFile(const DijkstraResult& result,
                             const std::string& out_path)
{
    ofstream f(out_path);
    if (!f.is_open()) {
        cerr << "[Dijkstra] Cannot write " << out_path << "\n";
        return;
    }
    for (long long nid : result.path) {
        f << nid << "\n";
    }
    f << "TOTAL_DISTANCE: " << result.total_distance << "\n";
    f << "TOTAL_TIME: "     << result.total_time_s   << "\n";
    cout << "[Dijkstra] Path written to " << out_path << "\n";
}
