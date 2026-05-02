#include <iostream>
#include <cstring>
#include "hash.h"

HashTable* createHashTable() {
    HashTable* ht = new HashTable;
    ht->size = HASH_SIZE;
    ht->table = new HashEntry[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; i++)
        ht->table[i].occupied = false;
    return ht;
}

// Hash function: sum of ASCII * position
static int hashFunction(HashTable* ht, const char* name) {
    int hash = 0;
    for (int i = 0; name[i]; i++)
        hash = (hash * 31 + name[i]) % ht->size;
    return hash;
}

void hashInsert(HashTable* ht, const char* name, int nodeId) {
    int idx = hashFunction(ht, name);
    // Linear probing for collision
    while (ht->table[idx].occupied) {
        if (strcmp(ht->table[idx].name, name) == 0) {
            ht->table[idx].nodeId = nodeId;  // update
            return;
        }
        idx = (idx + 1) % ht->size;
    }
    strncpy(ht->table[idx].name, name, MAX_NAME);
    ht->table[idx].name[MAX_NAME - 1] = '\0';
    ht->table[idx].nodeId = nodeId;
    ht->table[idx].occupied = true;
}

int hashSearch(HashTable* ht, const char* name) {
    int idx = hashFunction(ht, name);
    int start = idx;
    while (ht->table[idx].occupied) {
        if (strcmp(ht->table[idx].name, name) == 0)
            return ht->table[idx].nodeId;
        idx = (idx + 1) % ht->size;
        if (idx == start) break;
    }
    return -1;  // not found
}

void hashDelete(HashTable* ht, const char* name) {
    int idx = hashFunction(ht, name);
    int start = idx;
    while (ht->table[idx].occupied) {
        if (strcmp(ht->table[idx].name, name) == 0) {
            ht->table[idx].occupied = false;

            int next = (idx + 1) % ht->size;
            while (ht->table[next].occupied) {
                char reinsertName[MAX_NAME];
                int reinsertId = ht->table[next].nodeId;
                strncpy(reinsertName, ht->table[next].name, MAX_NAME);
                reinsertName[MAX_NAME - 1] = '\0';

                ht->table[next].occupied = false;
                hashInsert(ht, reinsertName, reinsertId);
                next = (next + 1) % ht->size;
            }
            return;
        }

        idx = (idx + 1) % ht->size;
        if (idx == start) break;
    }
}

void freeHashTable(HashTable* ht) {
    delete[] ht->table;
    delete ht;
}
