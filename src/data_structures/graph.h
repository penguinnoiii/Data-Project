#ifndef GRAPH_H
#define GRAPH_H

#define MAX_CODE 24
#define MAX_NAME 128
#define MAX_TYPE 24
#define INF 999999

struct Edge {
    int dest;
    int weight;
    Edge* next;
};

struct Node {
    int id;
    char code[MAX_CODE];
    char name[MAX_NAME];
    int imageX;
    int imageY;
    bool hasImagePosition;
    char type[MAX_TYPE];
    Edge* edges;
    Node* next;
};

struct Graph {
    Node* head;
    int nodeCount;
};

Graph* createGraph();
void addLocation(Graph* g, const char* name);
void addLocationWithPosition(Graph* g, const char* code, const char* name, int imageX, int imageY, const char* type);
void addPath(Graph* g, const char* src, const char* dest, int weight);
bool loadGraphFromFile(Graph* g, const char* filename);
void deleteLocation(Graph* g, const char* name);
void deletePath(Graph* g, const char* src, const char* dest);
void displayGraph(Graph* g);
Node* findNode(Graph* g, const char* name);
void freeGraph(Graph* g);

#endif
