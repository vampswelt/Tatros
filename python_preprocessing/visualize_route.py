#!/usr/bin/env python3
"""
visualize_route.py
------------------
Reads nodes.csv, edges.csv, and the path output from the C++ Dijkstra binary,
then renders an interactive Folium map saved as route_map.html.

Usage:
    python visualize_route.py --nodes nodes.csv --edges edges.csv \
                              --path path_output.txt [--out route_map.html]

The C++ binary should write path_output.txt with one node_id per line:
    <source_node_id>
    <node_id_1>
    <node_id_2>
    ...
    <destination_node_id>
    TOTAL_DISTANCE: <metres>
    TOTAL_TIME: <seconds>
"""

import argparse
import csv
import os
import sys

try:
    import folium
except ImportError:
    print("[ERROR] folium not found. Install via: pip install folium")
    sys.exit(1)


# ---------------------------------------------------------------------------
# Loaders
# ---------------------------------------------------------------------------
def load_nodes(path: str) -> dict:
    """Returns {node_id(int): (lat, lon)}"""
    nodes = {}
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            nodes[int(row["node_id"])] = (float(row["lat"]), float(row["lon"]))
    return nodes


def load_path(path: str):
    """
    Returns (list_of_node_ids, total_distance_m, total_time_s).
    Handles both plain node-per-line and annotated formats.
    """
    node_ids = []
    total_distance = None
    total_time = None

    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            if line.startswith("TOTAL_DISTANCE:"):
                try:
                    total_distance = float(line.split(":")[1].strip())
                except ValueError:
                    pass
            elif line.startswith("TOTAL_TIME:"):
                try:
                    total_time = float(line.split(":")[1].strip())
                except ValueError:
                    pass
            else:
                try:
                    node_ids.append(int(line))
                except ValueError:
                    pass

    return node_ids, total_distance, total_time


# ---------------------------------------------------------------------------
# Map builder
# ---------------------------------------------------------------------------
def build_map(nodes: dict, path_ids: list, total_distance, total_time, out_file: str):
    if not path_ids:
        print("[ERROR] Path is empty – nothing to draw.")
        sys.exit(1)

    # Collect coordinates along path
    coords = []
    missing = []
    for nid in path_ids:
        if nid in nodes:
            coords.append(nodes[nid])
        else:
            missing.append(nid)

    if missing:
        print(f"[WARN] {len(missing)} node(s) in path not found in nodes.csv: {missing[:5]} …")

    if len(coords) < 2:
        print("[ERROR] Not enough valid coordinates to draw a route.")
        sys.exit(1)

    # Centre map on midpoint of path
    mid = len(coords) // 2
    centre = coords[mid]
    m = folium.Map(location=centre, zoom_start=14, tiles="CartoDB positron")

    # ── Full graph background (light grey, optional) ──────────────────────
    # Skipped for performance on large graphs; enable if desired.

    # ── Route polyline ─────────────────────────────────────────────────────
    folium.PolyLine(
        locations=coords,
        color="#E63946",
        weight=5,
        opacity=0.9,
        tooltip="Shortest Path",
    ).add_to(m)

    # ── Source marker ──────────────────────────────────────────────────────
    folium.Marker(
        location=coords[0],
        tooltip=f"Source: Node {path_ids[0]}",
        icon=folium.Icon(color="green", icon="play", prefix="fa"),
    ).add_to(m)

    # ── Destination marker ─────────────────────────────────────────────────
    folium.Marker(
        location=coords[-1],
        tooltip=f"Destination: Node {path_ids[-1]}",
        icon=folium.Icon(color="red", icon="flag-checkered", prefix="fa"),
    ).add_to(m)

    # ── Intermediate waypoints (every ~10 % of path) ──────────────────────
    step = max(1, len(coords) // 10)
    for i in range(step, len(coords) - 1, step):
        folium.CircleMarker(
            location=coords[i],
            radius=4,
            color="#457B9D",
            fill=True,
            fill_opacity=0.7,
            tooltip=f"Node {path_ids[i]}",
        ).add_to(m)

    # ── Stats popup (bottom-left) ─────────────────────────────────────────
    dist_str = f"{total_distance / 1000:.2f} km" if total_distance is not None else "N/A"
    time_str = ""
    if total_time is not None:
        mins, secs = divmod(int(total_time), 60)
        hrs, mins = divmod(mins, 60)
        time_str = f"{hrs}h {mins}m {secs}s" if hrs else f"{mins}m {secs}s"
    else:
        time_str = "N/A"

    stats_html = f"""
    <div style="
        position: fixed; bottom: 30px; left: 30px; z-index: 1000;
        background: rgba(255,255,255,0.95);
        border: 2px solid #E63946;
        border-radius: 10px;
        padding: 14px 20px;
        font-family: 'Courier New', monospace;
        font-size: 13px;
        box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        min-width: 200px;
    ">
        <div style="font-weight:bold; font-size:15px; margin-bottom:8px; color:#E63946;">
            📍 Route Summary
        </div>
        <div><b>Nodes:</b> {len(path_ids)}</div>
        <div><b>Distance:</b> {dist_str}</div>
        <div><b>Est. Time:</b> {time_str}</div>
        <div style="margin-top:8px; color:#888; font-size:11px;">
            Source → Node {path_ids[0]}<br>
            Dest  → Node {path_ids[-1]}
        </div>
    </div>
    """
    m.get_root().html.add_child(folium.Element(stats_html))

    m.save(out_file)
    print(f"  Map saved → {os.path.abspath(out_file)}")
    print(f"   Nodes on path : {len(path_ids)}")
    print(f"   Distance       : {dist_str}")
    print(f"   Est. Time      : {time_str}")

def main():
    parser = argparse.ArgumentParser(description="Visualise Dijkstra route on a Folium map")
    parser.add_argument("--nodes", default="nodes.csv",      help="Path to nodes.csv")
    parser.add_argument("--edges", default="edges.csv",      help="Path to edges.csv (currently unused in viz)")
    parser.add_argument("--path",  default="path_output.txt", help="Path output file from C++ binary")
    parser.add_argument("--out",   default="route_map.html", help="Output HTML map file")
    args = parser.parse_args()

    print(f"Loading nodes from {args.nodes} …")
    nodes = load_nodes(args.nodes)
    print(f"  {len(nodes)} nodes loaded.")

    print(f"Loading path from {args.path} …")
    path_ids, total_dist, total_time = load_path(args.path)
    print(f"  {len(path_ids)} nodes in path.")

    print("Building map …")
    build_map(nodes, path_ids, total_dist, total_time, args.out)


if __name__ == "__main__":
    main()
