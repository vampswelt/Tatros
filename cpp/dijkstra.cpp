#include "dijkstra.h"
#include "cost_model.h"
#include <queue>
#include <limits>
#include <unordered_map>
#include <algorithm>

using namespace std;

DijkstraResult dijkstra(Graph& g, long long start, long long end, double slopeWeight) {

    unordered_map<long long, double> dist;
    unordered_map<long long, long long> parent;

    for (auto& p : g.nodes)
        dist[p.first] = 1e18;

    dist[start] = 0;

    priority_queue<
        pair<double, long long>,
        vector<pair<double, long long>>,
        greater<pair<double, long long>>
    > pq;

    pq.push({0, start});

    long long explored = 0;

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;

        explored++;

        if (u == end) break;

        for (auto& e : g.nodes[u].neighbors) {
            double cost = computeCost(e, slopeWeight);

            if (dist[e.dest] > dist[u] + cost) {
                dist[e.dest] = dist[u] + cost;
                parent[e.dest] = u;
                pq.push({dist[e.dest], e.dest});
            }
        }
    }

    vector<long long> path;

    if (dist[end] != 1e18) {
        for (long long cur = end; cur != start; cur = parent[cur])
            path.push_back(cur);

        path.push_back(start);
        reverse(path.begin(), path.end());
    }

    return {path, dist[end], explored};
}