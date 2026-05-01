#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
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
        if (strcmp(curr->name, name) == 0 || strcmp(curr->code, name) == 0) return curr;
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
    strncpy(newNode->code, name, MAX_CODE);
    newNode->code[MAX_CODE - 1] = '\0';
    strncpy(newNode->name, name, MAX_NAME);
    newNode->name[MAX_NAME - 1] = '\0';
    newNode->imageX = 0;
    newNode->imageY = 0;
    newNode->hasImagePosition = false;
    strncpy(newNode->type, "location", MAX_TYPE);
    newNode->type[MAX_TYPE - 1] = '\0';
    newNode->edges = nullptr;
    newNode->next = g->head;
    g->head = newNode;
    std::cout << "Added location: " << name << "\n";
}

void addLocationWithPosition(Graph* g, const char* code, const char* name, int imageX, int imageY, const char* type) {
    if (findNode(g, code) || findNode(g, name)) {
        std::cout << "Location '" << code << "' already exists.\n";
        return;
    }
    Node* newNode = new Node;
    newNode->id = g->nodeCount++;
    strncpy(newNode->code, code, MAX_CODE);
    newNode->code[MAX_CODE - 1] = '\0';
    strncpy(newNode->name, name, MAX_NAME);
    newNode->name[MAX_NAME - 1] = '\0';
    newNode->imageX = imageX;
    newNode->imageY = imageY;
    newNode->hasImagePosition = true;
    strncpy(newNode->type, type, MAX_TYPE);
    newNode->type[MAX_TYPE - 1] = '\0';
    newNode->edges = nullptr;
    newNode->next = g->head;
    g->head = newNode;
    std::cout << "Added location: " << code << " - " << name << "\n";
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
        PATHS,
        MAP_IMAGE
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

        if (line == "[MapImage]") {
            section = MAP_IMAGE;
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

        if (section == MAP_IMAGE) {
            continue;
        }

        if (section == LOCATIONS) {
            if (line.find('|') == std::string::npos) {
                addLocation(g, line.c_str());
                continue;
            }

            std::stringstream parser(line);
            std::string code;
            std::string name;
            std::string imageXText;
            std::string imageYText;
            std::string type;
            std::getline(parser, code, '|');
            std::getline(parser, name, '|');
            std::getline(parser, imageXText, '|');
            std::getline(parser, imageYText, '|');
            std::getline(parser, type, '|');

            try {
                addLocationWithPosition(g, code.c_str(), name.c_str(), std::stoi(imageXText),
                                        std::stoi(imageYText), type.empty() ? "location" : type.c_str());
            } catch (...) {
                std::cout << "Skipped invalid location entry: " << line << "\n";
            }
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
    Node* target = findNode(g, name);
    if (!target) {
        std::cout << "Location not found.\n";
        return;
    }

    int deletedId = target->id;

    for (Node* curr = g->head; curr; curr = curr->next) {
        Edge** edge = &curr->edges;
        while (*edge) {
            if ((*edge)->dest == deletedId) {
                Edge* removed = *edge;
                *edge = (*edge)->next;
                delete removed;
            } else {
                if ((*edge)->dest > deletedId) {
                    (*edge)->dest--;
                }
                edge = &((*edge)->next);
            }
        }
    }

    Node** curr = &g->head;
    while (*curr && *curr != target) {
        curr = &((*curr)->next);
    }
    if (*curr) {
        *curr = target->next;
    }

    Edge* edge = target->edges;
    while (edge) {
        Edge* removed = edge;
        edge = edge->next;
        delete removed;
    }
    delete target;

    for (Node* currNode = g->head; currNode; currNode = currNode->next) {
        if (currNode->id > deletedId) {
            currNode->id--;
        }
    }
    g->nodeCount--;

    std::cout << "Deleted location: " << name << "\n";
}

void deletePath(Graph* g, const char* src, const char* dest) {
    Node* srcNode = findNode(g, src);
    Node* destNode = findNode(g, dest);
    if (!srcNode || !destNode) {
        std::cout << "Location not found.\n";
        return;
    }

    auto removeEdge = [](Node* node, int destId) {
        bool removedAny = false;
        Edge** edge = &node->edges;
        while (*edge) {
            if ((*edge)->dest == destId) {
                Edge* removed = *edge;
                *edge = (*edge)->next;
                delete removed;
                removedAny = true;
            } else {
                edge = &((*edge)->next);
            }
        }
        return removedAny;
    };

    bool removed = removeEdge(srcNode, destNode->id);
    removed = removeEdge(destNode, srcNode->id) || removed;

    if (removed) {
        std::cout << "Deleted path: " << src << " <-> " << dest << "\n";
    } else {
        std::cout << "Path not found.\n";
    }
}

void displayGraph(Graph* g) {
    if (!g->head) {
        std::cout << "Map is empty.\n";
        return;
    }
    std::cout << "\n=== Campus Map ===\n";
    Node* curr = g->head;
    while (curr) {
        std::cout << "[" << curr->code << " " << curr->name << "]";
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
