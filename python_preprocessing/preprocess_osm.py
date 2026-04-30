

import os
import sys
import math
import csv

try:
    import osmium
except ImportError:
    print("[ERROR] osmium not found. Install via: pip install osmium")
    sys.exit(1)

def haversine(lat1, lon1, lat2, lon2):
    R = 6_371_000 
    phi1, phi2 = math.radians(lat1), math.radians(lat2)
    dphi = math.radians(lat2 - lat1)
    dlam = math.radians(lon2 - lon1)
    a = math.sin(dphi / 2) ** 2 + math.cos(phi1) * math.cos(phi2) * math.sin(dlam / 2) ** 2
    return R * 2 * math.atan2(math.sqrt(a), math.sqrt(1 - a))

ROAD_SPEEDS = {
    "motorway": 120, "motorway_link": 80,
    "trunk": 100, "trunk_link": 70,
    "primary": 80,  "primary_link": 60,
    "secondary": 60, "secondary_link": 50,
    "tertiary": 50,  "tertiary_link": 40,
    "unclassified": 40,
    "residential": 30,
    "service": 20,
    "living_street": 10,
    "pedestrian": 5,
    "track": 20,
    "road": 40,
}

ROUTABLE_ROADS = set(ROAD_SPEEDS.keys())


class WayNodeCollector(osmium.SimpleHandler):
    def __init__(self):
        super().__init__()
        self.used_node_ids: set = set()
        self.ways: list = []

    def way(self, w):
        highway = w.tags.get("highway", "")
        if highway not in ROUTABLE_ROADS:
            return
        node_ids = [n.ref for n in w.nodes]
        if len(node_ids) < 2:
            return
        for nid in node_ids:
            self.used_node_ids.add(nid)

        oneway_tag = w.tags.get("oneway", "no")
        oneway = oneway_tag in ("yes", "1", "true")
        if oneway_tag == "-1":
            node_ids = list(reversed(node_ids))
            oneway = True

        max_speed_tag = w.tags.get("maxspeed", "")
        try:
            max_speed = int("".join(filter(str.isdigit, max_speed_tag))) if max_speed_tag else 0
        except ValueError:
            max_speed = 0
        if max_speed == 0:
            max_speed = ROAD_SPEEDS.get(highway, 40)

        self.ways.append({
            "node_ids": node_ids,
            "highway": highway,
            "max_speed": max_speed,
            "oneway": oneway,
        })

class NodeCoordCollector(osmium.SimpleHandler):
    def __init__(self, used_ids: set):
        super().__init__()
        self.used_ids = used_ids
        self.coords: dict = {} 

    def node(self, n):
        if n.id in self.used_ids:
            self.coords[n.id] = (n.location.lat, n.location.lon)

def preprocess(osm_file: str, out_dir: str = "."):
    if not os.path.exists(osm_file):
        print(f"[ERROR] OSM file not found: {osm_file}")
        sys.exit(1)

    os.makedirs(out_dir, exist_ok=True)
    nodes_path = os.path.join(out_dir, "nodes.csv")
    edges_path = os.path.join(out_dir, "edges.csv")

    print(f"[1/3] Scanning ways in {osm_file} …")
    wc = WayNodeCollector()
    wc.apply_file(osm_file)
    print(f"Found {len(wc.ways)} routable ways, {len(wc.used_node_ids)} unique node refs.")

    print("[2/3] Collecting node coordinates …")
    nc = NodeCoordCollector(wc.used_node_ids)
    nc.apply_file(osm_file)
    print(f"      Resolved coordinates for {len(nc.coords)} nodes.")

    print("[3/3] Writing CSV files …")
    # nodes.csv
    with open(nodes_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["node_id", "lat", "lon"])
        for nid, (lat, lon) in nc.coords.items():
            writer.writerow([nid, f"{lat:.7f}", f"{lon:.7f}"])

    edge_count = 0
    with open(edges_path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["from_node", "to_node", "distance_m", "road_type", "max_speed_kmh", "oneway"])
        for way in wc.ways:
            nids = way["node_ids"]
            for i in range(len(nids) - 1):
                u, v = nids[i], nids[i + 1]
                if u not in nc.coords or v not in nc.coords:
                    continue
                lat1, lon1 = nc.coords[u]
                lat2, lon2 = nc.coords[v]
                dist = haversine(lat1, lon1, lat2, lon2)
                writer.writerow([u, v, f"{dist:.2f}", way["highway"], way["max_speed"], int(way["oneway"])])
                edge_count += 1
                if not way["oneway"]:
                    writer.writerow([v, u, f"{dist:.2f}", way["highway"], way["max_speed"], 0])
                    edge_count += 1

    print(f"\nDone!")
    print(f"nodes.csv → {len(nc.coords)} nodes")
    print(f"edges.csv → {edge_count} directed edges")
    print(f"Output directory: {os.path.abspath(out_dir)}")


if __name__ == "__main__":
    osm_input = sys.argv[1] if len(sys.argv) > 1 else "map.osm"
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "."
    preprocess(osm_input, output_dir)
