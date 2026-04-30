#include <iostream>
#include <string>
#include <stdexcept>
#include <unordered_map>

#include "graph.h"
#include "cost_model.h"
#include "dijkstra.h"
using namespace std;

static unordered_map<string, string>
parseArgs(int argc, char* argv[])
{
    unordered_map<string, string> args;
    for (int i = 1; i < argc - 1; i += 2) {
        string key = argv[i];
        if (key.size() > 2 && key[0] == '-' && key[1] == '-')
            args[key.substr(2)] = argv[i + 1];
    }
    return args;
}

int main(int argc, char* argv[]){

    auto args = parseArgs(argc, argv);

    string nodes_csv = args.count("nodes") ? args["nodes"] : "nodes.csv";
    string edges_csv = args.count("edges") ? args["edges"] : "edges.csv";
    string mode      = args.count("mode")  ? args["mode"]  : "time";
    string out_file  = args.count("out")   ? args["out"]   : "path_output.txt";
    double alpha = 0.5;
    if (args.count("alpha")) {
        try { alpha = stod(args["alpha"]); }
        catch (...) { alpha = 0.5; }
    }

    
    cout << "Loading graph …\n";
    Graph graph;
    if (!graph.loadFromCSV(nodes_csv, edges_csv)) {
        cerr << "[ERROR] Failed to load graph.\n";
        return 1;
    }
    cout << "Graph ready: " << graph.nodeCount() << " nodes, "
              << graph.edgeCount() << " edges.\n\n";

    
    CostModel* model = makeCostModel(mode, alpha);
    cout << "Cost model  : " << model->name() << "\n\n";

    long long src = -1, dst = -1;

    if (args.count("src")) {
        try { src = stoll(args["src"]); } catch (...) {}
    }
    if (args.count("dst")) {
        try { dst = stoll(args["dst"]); } catch (...) {}
    }

    while (src == -1 || !graph.hasNode(src)) {
        cout << "Enter SOURCE node ID: ";
        cin >> src;
        if (!graph.hasNode(src))
            cerr << "  [WARN] Node " << src << " not found. Try again.\n";
    }
    while (dst == -1 || !graph.hasNode(dst)) {
        cout << "Enter DESTINATION node ID: ";
        cin >> dst;
        if (!graph.hasNode(dst))
            cerr << "  [WARN] Node " << dst << " not found. Try again.\n";
    }

    const Node* sNode = graph.getNode(src);
    const Node* dNode = graph.getNode(dst);
    cout << "\nSource : " << src
              << " (" << sNode->lat << ", " << sNode->lon << ")\n";
    cout << "Dest   : " << dst
              << " (" << dNode->lat << ", " << dNode->lon << ")\n\n";

    Dijkstra dijkstra(graph, *model);
    DijkstraResult result = dijkstra.run(src, dst);

    if (!result.found) {
        cerr << "\n[ERROR] No path found between " << src << " and " << dst << ".\n";
        delete model;
        return 2;
    }

    double dist_km = result.total_distance / 1000.0;
    int total_s    = static_cast<int>(result.total_time_s);
    int hrs  = total_s / 3600;
    int mins = (total_s % 3600) / 60;
    int secs = total_s % 60;
    cout << "Path nodes : " << result.path.size() << "\n";
    cout << "Distance   : " << dist_km << " km\n";
    if (hrs > 0)
        cout << "Est. time  : " << hrs << "h " << mins << "m " << secs << "s\n";
    else
        cout << "Est. time  : " << mins << "m " << secs << "s\n";

    const auto& path = result.path;
    size_t show = min<size_t>(5, path.size());
    cout << "Path preview (first " << show << " nodes): ";
    for (size_t i = 0; i < show; ++i)
        cout << path[i] << (i + 1 < show ? " → " : "");
    if (path.size() > show * 2)
        cout << " → … → ";
    if (path.size() > show) {
        for (size_t i = path.size() - show; i < path.size(); ++i)
            cout << path[i] << (i + 1 < path.size() ? " → " : "");
    }
    cout << "\n\n";

    Dijkstra::writePathFile(result, out_file);

    cout << "\nRun the visualiser:\n";
    cout << "  python3 python_preprocessing/visualize_route.py"
              << " --path " << out_file << "\n\n";

    delete model;
    return 0;
}
