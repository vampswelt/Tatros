#include "graph.h"
#include "dijkstra.h"
#include <iostream>
#include <fstream>

using namespace std;

int main() {

    Graph g;
    g.loadNodes("../data/nodes.csv");
    g.loadEdges("../data/edges.csv");

    long long s, t;
    cin >> s >> t;

    auto result = dijkstra(g, s, t, 1.0);

    ofstream f("../data/path.txt");

    for (auto id : result.path)
        f << id << "\n";

    cout << "Cost: " << result.totalCost << endl;
}