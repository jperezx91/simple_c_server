#define MAX_SIZE 100
#pragma once
typedef struct HashNode {
    char *key;
    char *value;
    int sz;
    struct HashNode *next;
}HashNode;

typedef struct HashMap {
    int size;
    HashNode *table[MAX_SIZE];
}HashMap;

unsigned int hash(const char *key);
HashNode *create_node(const char *key, char *value, int sz);
void insert(HashMap *map, const char *key, char *value, int sz);
HashNode *get(HashMap *map, const char *key);
void delete(HashMap *map, const char *key);
HashMap *create_map();
void limpiar(HashMap *map);