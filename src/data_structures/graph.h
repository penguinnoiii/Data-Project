#ifndef GRAPH_H
#define GRAPH_H

#define MAX_NAME 50
#define INF 999999

struct Edge {
    int dest;
    int weight;
    Edge* next;
};

struct Node {
    int id;
    char name[MAX_NAME];
    Edge* edges;
    Node* next;
};

struct Graph {
    Node* head;
    int nodeCount;
};

Graph* createGraph();
void addLocation(Graph* g, const char* name);
void addPath(Graph* g, const char* src, const char* dest, int weight);
bool loadGraphFromFile(Graph* g, const char* filename);
void deleteLocation(Graph* g, const char* name);
void deletePath(Graph* g, const char* src, const char* dest);
void displayGraph(Graph* g);
Node* findNode(Graph* g, const char* name);
void freeGraph(Graph* g);

#endif
