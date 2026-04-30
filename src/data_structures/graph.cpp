#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include "graph.h"

Graph* createGraph() {
    Graph* g = new Graph;
    g->head = nullptr;
    g->nodeCount = 0;
    return g;
}

Node* findNode(Graph* g, const char* name) {
    Node* curr = g->head;
    while (curr) {
        if (strcmp(curr->name, name) == 0) return curr;
        curr = curr->next;
    }
    return nullptr;
}

void addLocation(Graph* g, const char* name) {
    if (findNode(g, name)) {
        std::cout << "Location '" << name << "' already exists.\n";
        return;
    }
    Node* newNode = new Node;
    newNode->id = g->nodeCount++;
    strncpy(newNode->name, name, MAX_NAME);
    newNode->name[MAX_NAME - 1] = '\0';
    newNode->edges = nullptr;
    newNode->next = g->head;
    g->head = newNode;
    std::cout << "Added location: " << name << "\n";
}

void addPath(Graph* g, const char* src, const char* dest, int weight) {
    Node* srcNode = findNode(g, src);
    Node* destNode = findNode(g, dest);
    if (!srcNode || !destNode) {
        std::cout << "Location not found.\n";
        return;
    }

    // Add edge src -> dest
    Edge* e1 = new Edge;
    e1->dest = destNode->id;
    e1->weight = weight;
    e1->next = srcNode->edges;
    srcNode->edges = e1;

    // Add edge dest -> src (undirected)
    Edge* e2 = new Edge;
    e2->dest = srcNode->id;
    e2->weight = weight;
    e2->next = destNode->edges;
    destNode->edges = e2;

    std::cout << "Added path: " << src << " <-> " << dest << " (distance: " << weight << ")\n";
}

bool loadGraphFromFile(Graph* g, const char* filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cout << "Could not open data file: " << filename << "\n";
        return false;
    }

    enum Section {
        NONE,
        LOCATIONS,
        PATHS
    };

    Section section = NONE;
    std::string line;

    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "[Locations]") {
            section = LOCATIONS;
            continue;
        }

        if (line == "[Paths]") {
            section = PATHS;
            continue;
        }

        if (section == LOCATIONS) {
            addLocation(g, line.c_str());
            continue;
        }

        if (section == PATHS) {
            size_t firstSep = line.find('|');
            size_t secondSep = line.rfind('|');

            if (firstSep == std::string::npos || secondSep == std::string::npos || firstSep == secondSep) {
                std::cout << "Skipped invalid path entry: " << line << "\n";
                continue;
            }

            std::string src = line.substr(0, firstSep);
            std::string dest = line.substr(firstSep + 1, secondSep - firstSep - 1);
            std::string weightText = line.substr(secondSep + 1);

            try {
                int weight = std::stoi(weightText);
                addPath(g, src.c_str(), dest.c_str(), weight);
            } catch (...) {
                std::cout << "Skipped invalid path entry: " << line << "\n";
            }
        }
    }

    return true;
}

void deleteLocation(Graph* g, const char* name) {
    // TODO (Member 1): Remove node and all edges connected to it
    // Steps:
    // 1. Find node to delete
    // 2. Remove all edges pointing TO this node from other nodes
    // 3. Remove the node itself from the linked list
    // 4. Free memory
    std::cout << "TODO: deleteLocation\n";
}

void deletePath(Graph* g, const char* src, const char* dest) {
    // TODO (Member 1): Remove edge between src and dest (both directions)
    // Steps:
    // 1. Find srcNode, remove edge pointing to dest
    // 2. Find destNode, remove edge pointing to src
    // 3. Free edge memory
    std::cout << "TODO: deletePath\n";
}

void displayGraph(Graph* g) {
    if (!g->head) {
        std::cout << "Map is empty.\n";
        return;
    }
    std::cout << "\n=== Campus Map ===\n";
    Node* curr = g->head;
    while (curr) {
        std::cout << "[" << curr->name << "]";
        Edge* e = curr->edges;
        while (e) {
            // Find node name by id
            Node* neighbor = g->head;
            while (neighbor && neighbor->id != e->dest) neighbor = neighbor->next;
            if (neighbor)
                std::cout << " -> " << neighbor->name << "(" << e->weight << ")";
            e = e->next;
        }
        std::cout << "\n";
        curr = curr->next;
    }
}

void freeGraph(Graph* g) {
    Node* curr = g->head;
    while (curr) {
        Edge* e = curr->edges;
        while (e) {
            Edge* tmp = e;
            e = e->next;
            delete tmp;
        }
        Node* tmp = curr;
        curr = curr->next;
        delete tmp;
    }
    delete g;
}
