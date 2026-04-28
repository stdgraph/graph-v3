# Benchmark Data

This directory contains real-world graph data files used as validation
fixtures in the Dijkstra benchmark suite (Phase 0.2).  The files are
large and are therefore **not committed** to the repository (see
`.gitignore`).  Use the instructions below to download them before
running the real-world validation benchmarks.

---

## Required files

| Filename | Vertices | Edges | Source | Description |
|----------|----------|-------|--------|-------------|
| `roadNet-CA.txt` | 1,965,206 | 5,533,214 | SNAP | California road network – classic Dijkstra benchmark, planar/spatial |
| `web-Google.txt` | 875,713 | 5,105,039 | SNAP | Web-link graph – mixed degree distribution |

---

## Download instructions

### Stanford SNAP graphs

```bash
# Create the data directory if it does not already exist
mkdir -p benchmark/data

# California road network
curl -L "https://snap.stanford.edu/data/roadNet-CA.txt.gz" \
  | gunzip > benchmark/data/roadNet-CA.txt

# Google web graph
curl -L "https://snap.stanford.edu/data/web-Google.txt.gz" \
  | gunzip > benchmark/data/web-Google.txt
```

Alternatively, download from <https://snap.stanford.edu/data/> and place
the decompressed `.txt` files in this directory.

---

## File format

SNAP edge-list files use the following format:

```
# Comment lines start with '#'
<source_id>\t<target_id>
```

Vertex ids are 0-based integers.  The benchmark loader skips comment
lines and treats each remaining line as a directed edge.

---

## Loader

The fixture helper `benchmark/algorithms/dijkstra_fixtures.hpp` will
gain a `load_snap_graph()` function in Phase 0 that reads these files
and returns a sorted `edge_list`.  For now, running the real-world
benchmarks requires that the files are present; if they are absent the
corresponding benchmark cases are skipped at runtime with a message.

---

## License / attribution

The SNAP graphs are distributed by the Stanford Network Analysis Project
under their respective licences.  Please cite the original dataset when
publishing results.

- Jure Leskovec and Andrej Krevl.  *SNAP Datasets: Stanford Large Network
  Dataset Collection*, <http://snap.stanford.edu/data>, June 2014.
