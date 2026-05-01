# CPE112 Final Project

**Course:** CPE112 Programming with Data Structures  
**Semester:** 2/2025  
**Deadline:** May 14, 2026

## Team Members

| Name | GitHub Username |
|------|----------------|
| (ชื่อ) | (username) |
| (ชื่อ) | (username) |
| (ชื่อ) | (username) |

## Project Description

(อธิบายโปรเจกต์)

## Data Structures Used

- (เช่น Graph — เพราะ...)
- (เช่น Hash Table — เพราะ...)

## How to Build & Run

Install Raylib first:

```bash
brew install raylib pkg-config
```

```bash
make
make run
```

The Raylib map uses `assets/Masterplan  KMUTT.jpg` as the background. Node
positions in `data/campus_map.txt` are original image pixel coordinates, and
path weights are approximate meters derived from the map scale bar.

Or manually:
```bash
g++ -Wall -std=c++17 -Isrc -o build/program src/*.cpp src/data_structures/*.cpp -lraylib
./build/program
```

## Project Structure

```
├── src/
│   ├── main.cpp
│   ├── menu.cpp / menu.h
│   └── data_structures/      # Data structure implementations
├── docs/                     # Report & diagrams
├── assets/                   # GUI assets (if any)
├── tests/                    # Test cases
├── Makefile
└── README.md
```
