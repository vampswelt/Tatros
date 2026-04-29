import osmium
import csv
import os

# -------------------------
# CONFIG
# -------------------------
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
OSM_FILE = os.path.join(BASE_DIR, "..", "map.osm")

DATA_DIR = os.path.join(BASE_DIR, "data")
os.makedirs(DATA_DIR, exist_ok=True)

NODES_CSV = os.path.join(DATA_DIR, "nodes.csv")
EDGES_CSV = os.path.join(DATA_DIR, "edges.csv")

print("CWD:", os.getcwd())
print("Looking for OSM file:", OSM_FILE)

if not os.path.exists(OSM_FILE):
    print("❌ map.osm not found!")
    print("Place map.osm in TatrosP.3 folder")
    exit()

# -------------------------
# HANDLER
# -------------------------
class RoadHandler(osmium.SimpleHandler):
    def __init__(self):
        super().__init__()
        self.nodes = {}
        self.edges = []

    def node(self, n):
        if n.location.valid():
            self.nodes[n.id] = (n.location.lat, n.location.lon)

    def way(self, w):
        if "highway" not in w.tags:
            return

        refs = [n.ref for n in w.nodes]

        for i in range(len(refs) - 1):
            a = refs[i]
            b = refs[i + 1]

            if a in self.nodes and b in self.nodes:
                self.edges.append((a, b))


# -------------------------
# RUN PARSER
# -------------------------
print("Loading OSM file...")

handler = RoadHandler()
handler.apply_file(OSM_FILE)

print("Parsing complete")
print("Nodes:", len(handler.nodes))
print("Edges:", len(handler.edges))

# -------------------------
# WRITE NODES
# -------------------------
print("Writing nodes.csv...")

with open(NODES_CSV, "w", newline="", encoding="utf-8") as f:
    writer = csv.writer(f)
    writer.writerow(["id", "lat", "lon"])

    for nid, (lat, lon) in handler.nodes.items():
        writer.writerow([nid, lat, lon])

# -------------------------
# WRITE EDGES
# -------------------------
print("Writing edges.csv...")

with open(EDGES_CSV, "w", newline="", encoding="utf-8") as f:
    writer = csv.writer(f)
    writer.writerow(["src", "dst"])

    for src, dst in handler.edges:
        writer.writerow([src, dst])

print("✅ DONE SUCCESSFULLY")
print("Files saved in:", DATA_DIR)