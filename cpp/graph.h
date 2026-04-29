#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <unordered_map>
using namespace std;

struct Edge {
    long long dest;
    double dist;
    double slope;
};

struct Node {
    long long id;
    double lat, lon, elev;
    vector<Edge> neighbors;
};

class Graph {
public:
    unordered_map<long long, Node> nodes;

    void addNode(long long id, double lat, double lon, double elev);
    void addEdge(long long s, long long d, double dist, double slope);

    void loadNodes(const string& file);
    void loadEdges(const string& file);
};

#endif