#include <stdio.h>
#include <stdlib.h>
#include "Hashmap.h"

typedef struct HashMap HashMap;
int enlarge_called = 0;

struct HashMap {
    Pair **buckets;
    long size; // cantidad de datos/pairs en la tabla
    long capacity; // capacidad de la tabla
    long current; // indice del ultimo dato accedido
};

Pair *createPair(int key, void *value) {
    Pair *new = (Pair *)malloc(sizeof(Pair));
    new->key = key;
    new->value = value;
    return new;
}

long hash(int key, long capacity) {
    // Simple hash function for integers
    return (key % capacity);
}

int is_equal(int key1, int key2) {
    return key1 == key2;
}

void insertMap(HashMap *map, int key, void *value) {
    if (map == NULL) return;

    long pos = hash(key, map->capacity);

    // Linear probing for collision resolution
    while (map->buckets[pos] != NULL) {
        if (is_equal(map->buckets[pos]->key, key)) {
            // Key already exists, update value
            map->buckets[pos]->value = value;
            return;
        }
        pos = (pos + 1) % map->capacity;
    }

    Pair *nuevo = createPair(key, value);
    map->buckets[pos] = nuevo;
    map->current = pos;
    map->size++;

    // Check load factor and enlarge if necessary
    if ((double)map->size / map->capacity > 0.75) {
        enlarge(map);
    }
}

void enlarge(HashMap *map) {
    enlarge_called = 1; // No borrar (testing purposes)
    Pair **old_buckets = map->buckets;
    long old_capacity = map->capacity;

    map->capacity *= 2;
    map->buckets = (Pair **)calloc(map->capacity, sizeof(Pair *));
    map->size = 0;

    // Reinsert valid pairs
    for (long k = 0; k < old_capacity; k++) {
        if (old_buckets[k] != NULL && old_buckets[k]->key != -1) {
            insertMap(map, old_buckets[k]->key, old_buckets[k]->value);
            free(old_buckets[k]);
        }
    }
    free(old_buckets);
}

HashMap *createMap(long capacity) {
    HashMap *mapa = (HashMap *)malloc(sizeof(HashMap));
    mapa->buckets = (Pair **)calloc(capacity, sizeof(Pair *));
    mapa->size = 0;
    mapa->capacity = capacity;
    mapa->current = -1;
    return mapa;
}

void eraseMap(HashMap *map, int key) {
    if (map == NULL) return;

    long pos = hash(key, map->capacity);
    while (map->buckets[pos] != NULL) {
        if (is_equal(map->buckets[pos]->key, key)) {
            free(map->buckets[pos]);
            map->buckets[pos] = NULL;
            map->size--;
            return;
        }
        pos = (pos + 1) % map->capacity;
    }
}

Pair *searchMap(HashMap *map, int key) {
    if (map == NULL) return NULL;

    long pos = hash(key, map->capacity);
    while (map->buckets[pos] != NULL) {
        if (is_equal(map->buckets[pos]->key, key)) {
            map->current = pos;
            return map->buckets[pos];
        }
        pos = (pos + 1) % map->capacity;
    }
    return NULL;
}

Pair *firstMap(HashMap *map) {
    if (map == NULL || map->size == 0) return NULL;

    for (long k = 0; k < map->capacity; k++) {
        if (map->buckets[k] != NULL && map->buckets[k]->key != -1) {
            map->current = k;
            return map->buckets[k];
        }
    }
    return NULL;
}

Pair *nextMap(HashMap *map) {
    if (map == NULL || map->current < 0 || map->current >= map->capacity - 1) return NULL;

    for (long k = map->current + 1; k < map->capacity; k++) {
        if (map->buckets[k] != NULL && map->buckets[k]->key != -1) {
            map->current = k;
            return map->buckets[k];
        }
    }
    return NULL;
}

void hashmap_clean(HashMap *map) {
    if (map == NULL) return;

    for (long k = 0; k < map->capacity; k++) {
        if (map->buckets[k] != NULL) {
            free(map->buckets[k]);
            map->buckets[k] = NULL;
        }
    }
    free(map->buckets);
    free(map);
}