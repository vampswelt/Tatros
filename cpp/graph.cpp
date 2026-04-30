
#include "graph.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
using namespace std;

const vector<Edge> Graph::EMPTY_EDGES;


static vector<std::string> splitCSV(const string& line) {
    vector<string> fields;
    string field;
    bool inQuote = false;
    for (char ch : line) {
        if (ch == '"') { inQuote = !inQuote; }
        else if (ch == ',' && !inQuote) { fields.push_back(field); field.clear(); }
        else { field += ch; }
    }
    fields.push_back(field);
    return fields;
}


bool Graph::loadFromCSV(const std::string& nodes_csv,
                        const std::string& edges_csv)
{
    
    {
        ifstream f(nodes_csv);
        if (!f.is_open()) {
            std::cerr << "[Graph] Cannot open " << nodes_csv << "\n";
            return false;
        }
        string line;
        getline(f, line); 
        while (getline(f, line)) {
            if (line.empty()) continue;
            auto cols = splitCSV(line);
            if (cols.size() < 3) continue;
            Node n;
            n.id  = stoll(cols[0]);
            n.lat = stod(cols[1]);
            n.lon = stod(cols[2]);
            nodes_[n.id] = n;
            node_ids_.push_back(n.id);
        }
        cout << "[Graph] Loaded " << nodes_.size() << " nodes.\n";
    }

    
    {
        ifstream f(edges_csv);
        if (!f.is_open()) {
            cerr << "[Graph] Cannot open " << edges_csv << "\n";
            return false;
        }
        string line;
        getline(f, line); // header
        while (getline(f, line)) {
            if (line.empty()) continue;
            auto cols = splitCSV(line);
            if (cols.size() < 6) continue;
            long long from       = std::stoll(cols[0]);
            long long to         = std::stoll(cols[1]);
            double    dist       = std::stod(cols[2]);
            std::string road_type = cols[3];
            int       speed      = std::stoi(cols[4]);
            bool      oneway     = (cols[5] == "1");

            Edge e;
            e.to           = to;
            e.distance_m   = dist;
            e.cost         = dist;   
            e.road_type    = road_type;
            e.max_speed_kmh = speed;
            e.oneway       = oneway;

            adj_[from].push_back(e);
            ++edge_count_;
        }
        cout << "[Graph] Loaded " << edge_count_ << " directed edges.\n";
    }

    return true;
}

const Node* Graph::getNode(long long id) const {
    auto it = nodes_.find(id);
    return (it != nodes_.end()) ? &it->second : nullptr;
}

const vector<Edge>& Graph::getNeighbours(long long id) const {
    auto it = adj_.find(id);
    return (it != adj_.end()) ? it->second : EMPTY_EDGES;
}

const vector<long long>& Graph::getAllNodeIds() const {
    return node_ids_;
}

bool Graph::hasNode(long long id) const {
    return nodes_.count(id) > 0;
}
