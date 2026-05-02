#ifndef HASH_H
#define HASH_H

#define HASH_SIZE 257
#define MAX_NAME 128

struct HashEntry {
    char name[MAX_NAME];
    int nodeId;
    bool occupied;
};

struct HashTable {
    HashEntry* table;
    int size;
};

HashTable* createHashTable();
void hashInsert(HashTable* ht, const char* name, int nodeId);
int hashSearch(HashTable* ht, const char* name);  // returns nodeId or -1
void hashDelete(HashTable* ht, const char* name);
void freeHashTable(HashTable* ht);

#endif
