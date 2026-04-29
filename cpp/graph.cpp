#include "graph.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

void Graph::addNode(long long id, double lat, double lon, double elev) {
    nodes[id] = {id, lat, lon, elev, {}};
}

void Graph::addEdge(long long src, long long dst, double dist, double slope) {
    if (nodes.count(src) && nodes.count(dst)) {
        nodes[src].neighbors.push_back({dst, dist, slope});
        nodes[dst].neighbors.push_back({src, dist, slope});
    }
}

void Graph::loadNodes(const string& file) {
    ifstream f(file);
    string line;
    getline(f, line);

    while (getline(f, line)) {
        stringstream ss(line);
        string id, lat, lon, elev;

        getline(ss, id, ',');
        getline(ss, lat, ',');
        getline(ss, lon, ',');
        getline(ss, elev, ',');

        addNode(stoll(id), stod(lat), stod(lon), stod(elev));
    }

    cout << "Nodes loaded: " << nodes.size() << endl;
}

void Graph::loadEdges(const string& file) {
    ifstream f(file);
    string line;
    getline(f, line);

    int count = 0;

    while (getline(f, line)) {
        stringstream ss(line);
        string s, d, dist, slope;

        getline(ss, s, ',');
        getline(ss, d, ',');
        getline(ss, dist, ',');
        getline(ss, slope, ',');

        addEdge(stoll(s), stoll(d), stod(dist), stod(slope));
        count++;
    }

    cout << "Edges loaded: " << count << endl;
}