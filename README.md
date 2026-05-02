# Smart Campus Navigation

**Course:** CPE112 Programming with Data Structures  
**Semester:** 2/2025  
**Deadline:** May 14, 2026

## Team Members

| Name                 | GitHub Username |
|----------------------|-----------------|
| Pornnapat Poonpolsub | penguinnoiii    |
| (ชื่อ) | (username) |
| (ชื่อ) | (username) |

## Project Description

Smart Campus Navigation is a Raylib desktop application for finding routes
between locations on the KMUTT Bangmod campus. The app displays the KMUTT
master-plan image as the map background, places nodes on the correct map
locations, and uses graph search to calculate routes.

Users can:

- Search locations by building code or name, such as `N10` or `Library`.
- Click two map nodes to find the shortest path.
- Type two locations into the Shortest Path panel.
- Add/delete locations and connect paths from the GUI.
- Zoom in/out on the campus map and pan with right-drag.

## Data Structures Used

- **Graph:** stores campus locations as nodes and walkable connections as
  weighted edges. Dijkstra's algorithm uses this graph to find the shortest
  weighted path.
- **Hash Table:** maps location names and codes to node IDs for fast search.
- **Priority Queue:** supports Dijkstra's algorithm by selecting the next node
  with the smallest known distance.
- **Queue:** included for BFS/unweighted traversal support.

## Map Data

Map and graph data are stored in `data/campus_map.txt`.

- `[MapImage]` config points to `assets/Masterplan  KMUTT.jpg`.
- `[Locations]` rows use:

```txt
code|display_name|image_x|image_y|type
```

- `[Paths]` rows use:

```txt
from_code|to_code|weight_meters
```

Node coordinates are original image pixel coordinates. Path weights are
approximate meters calculated from the map scale bar (`409 px = 100 m`).

## Controls

- Mouse wheel over map: zoom in/out
- `+` / `-`: zoom in/out
- Right-drag on map: pan
- Reset view: return to full campus view
- Click two nodes: calculate shortest path
- Tab in a textbox: accept the highlighted autocomplete suggestion
- Up/Down in a textbox: change autocomplete selection

## How to Build & Run

Install Raylib first:

```bash
brew install raylib pkg-config
```

Build and run:

```bash
make
make run
```

Or manually:

```bash
mkdir -p build
g++ -Wall -std=c++17 -Isrc -o build/program src/*.cpp src/data_structures/*.cpp -lraylib
./build/program
```

## Project Structure

```txt
├── assets/
│   └── Masterplan  KMUTT.jpg
├── data/
│   └── campus_map.txt
├── src/
│   ├── main.cpp
│   ├── menu.cpp / menu.h
│   └── data_structures/
│       ├── algorithm.cpp / algorithm.h
│       ├── graph.cpp / graph.h
│       ├── hash.cpp / hash.h
│       └── queue.cpp / queue.h
├── tests/
├── Makefile
└── README.md
```
