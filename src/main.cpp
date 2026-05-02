#include "menu.h"
#include "data_structures/graph.h"
#include "data_structures/hash.h"

static void loadHashFromGraph(Graph* g, HashTable* ht) {
    Node* curr = g->head;
    while (curr) {
        hashInsert(ht, curr->name, curr->id);
        curr = curr->next;
    }
}

int main() {    
    Graph* g = createGraph();
    HashTable* ht = createHashTable();

    if (loadGraphFromFile(g, "data/campus_map.txt")) {
        loadHashFromGraph(g, ht);
    }

    showMenu(g, ht);

    freeGraph(g);
    freeHashTable(ht);
    return 0;
}
