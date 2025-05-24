#include "list.h"
#include "extra.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// Crea un nuevo grafo con un número fijo de nodos
Graph* graph_create(int num_nodos) {
    Graph* grafo = (Graph*)malloc(sizeof(Graph));
    grafo->nodos = (Escenario**)calloc(num_nodos, sizeof(Escenario*));
    grafo->num_nodos = num_nodos;
    return grafo;
}

// Crea un nuevo escenario (nodo del grafo)
Escenario* escenario_create(int id, char* nombre, char* descripcion, List* items, int arriba, int abajo, int izquierda, int derecha, int es_final) {
    Escenario* esc = (Escenario*)malloc(sizeof(Escenario));
    esc->id = id;
    esc->nombre = strdup(nombre);
    esc->descripcion = strdup(descripcion);
    esc->items = items;
    esc->conexiones[0] = arriba;
    esc->conexiones[1] = abajo;
    esc->conexiones[2] = izquierda;
    esc->conexiones[3] = derecha;
    esc->es_final = es_final;
    return esc;
}

// Libera un ítem
void item_free(Item* item) {
    if (item) {
        free(item->nombre);
        free(item);
    }
}

// Libera un escenario
void escenario_free(Escenario* esc) {
    if (esc) {
        free(esc->nombre);
        free(esc->descripcion);
        for (Item* item = list_first(esc->items); item != NULL; item = list_next(esc->items)) {
            item_free(item);
        }
        list_clean(esc->items);
        free(esc->items);
        free(esc);
    }
}

// liberar memoria del grafo cuando se quiera reiniciar la partida
void graph_free(Graph* g) {
    if (g) {
        for (int i = 0; i < g->num_nodos; i++) {
            if (g->nodos[i]) {
                escenario_free(g->nodos[i]);
            }
        }
        free(g->nodos);
        free(g);
    }
}