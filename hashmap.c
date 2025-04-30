#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hashmap.h"

unsigned int hash(const char *key)
{
    unsigned int hash = 0;
    int i = 0;
    while (key[i] != '\0') {
        hash = hash * 31 + key[i];
        i++;
    }
    return hash % MAX_SIZE;
}

HashNode *create_node(const char *key, char *value, int sz)
{
    HashNode *node = malloc(sizeof(HashNode));
    node->key = malloc(strlen(key) + 1);
    node->sz = sz;
    strcpy(node->key, key);
    node->value = value;
    node->next = NULL;
    node->children = NULL;
    node->nextChild = NULL;
    return node;
}

void limpiar(HashMap *map) {
    for (int i = 0; i < MAX_SIZE; i++) {
        HashNode *current = map->table[i];
        while (current != NULL) {
            HashNode *next = current->next;

            // Liberar memoria de key y value, verificando que no sean NULL
            if (current->key != NULL) {
                printf("Current key: %s\n", current->key);
                free(current->key);
                current->key = NULL;
            }
            if (current->value != NULL) {
                printf("Liberando memoria de value. %ld\n", sizeof(current->value));
                free(current->value);
                current->value = NULL;
            }
            
            // Liberar el nodo actual
            if(current != NULL)
                free(current);
            current = next;
           
        }

        // Marcar el índice como limpio
        map->table[i] = NULL;
    }
}

HashNode *insert(HashMap *map, const char *key, char *value, int sz, HashNode *parent)
{
    int index = hash(key);
    HashNode *node = create_node(key, value, sz);
    HashNode *output;
    if (map->table[index] == NULL) {
        map->table[index] = node;
        output = map->table[index];
    } else {
        HashNode *current = map->table[index];
        while (current->next != NULL) 
            current = current->next;
        
        current->next = node;
        output = current->next;
    }
    map->size++;
    if(parent != NULL)
    {
        
        if(parent->children == NULL)
        {
            parent->children = output;
        }else
        {
            HashNode *aux = parent->children;
            while(aux->nextChild != NULL)
                aux = aux->nextChild;
            aux->nextChild = output;
        }
    }
    return output;
}

HashNode *get(HashMap *map, const char *key)
{
    int index = hash(key);
    HashNode *current = map->table[index];
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void delete(HashMap *map, const char *key)
{
    int index = hash(key);
    HashNode *current = map->table[index];
    HashNode *previous = NULL;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (previous == NULL) 
                map->table[index] = current->next;
            else 
                previous->next = current->next;
            
            map->size--;
            return;
        }
        previous = current;
        current = current->next;
    }
    
}

HashMap *create_map()
{
    HashMap *map = malloc(sizeof(HashMap));
    map->size = 0;
    for (int i = 0; i < MAX_SIZE; i++)
        map->table[i] = NULL;
    
    return map;
}