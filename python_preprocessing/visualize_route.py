import folium
import csv
nodes = {}
with open("../data/nodes.csv") as f:
    r = csv.DictReader(f)
    for row in r:
        nodes[row["id"]] = (float(row["lat"]), float(row["lon"]))
path = []
with open("../data/path.txt") as f:
    path = [x.strip() for x in f if x.strip() in nodes]
coords = [nodes[p] for p in path]
m = folium.Map(location=coords[0], zoom_start=14)
folium.PolyLine(coords, color="blue", weight=5).add_to(m)
folium.Marker(coords[0], popup="START").add_to(m)
folium.Marker(coords[-1], popup="END").add_to(m)
for c in coords[::20]:
    folium.Circle(
        location=c,
        radius=80,
        color="red",
        fill=True,
        fill_opacity=0.2
    ).add_to(m)
m.save("output.html")
print("Map generated")