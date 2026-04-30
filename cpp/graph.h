#pragma once
v.

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

struct Node {
    long long id;
    double    lat;
    double    lon;
};


struct Edge {
    long long to;          
    double    distance_m;  
    double    cost;        
    string road_type;
    int       max_speed_kmh;
    bool      oneway;
};


class Graph {
public:
   
    bool loadFromCSV(const std::string& nodes_csv,
                     const std::string& edges_csv);

    
    const Node*              getNode(long long id) const;
    const std::vector<Edge>& getNeighbours(long long id) const;
    const std::vector<long long>& getAllNodeIds() const;

    size_t nodeCount() const { return nodes_.size(); }
    size_t edgeCount() const { return edge_count_;   }

    bool hasNode(long long id) const;

private:
    unordered_map<long long, Node>              nodes_;
    unordered_map<long long, vector<Edge>> adj_;
    vector<long long>                           node_ids_;   
    size_t edge_count_ = 0;

    static const vector<Edge> EMPTY_EDGES;
};
