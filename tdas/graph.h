
#ifndef GRAPH_H
#define GRAPH_H
#include "list.h"



Graph* graph_create(int num_nodos);
Escenario* escenario_create(int id, char* nombre, char* descripcion, List* items, int arriba, int abajo, int izquierda, int derecha, int es_final);
void item_free(Item* item);
void escenario_free(Escenario* esc);
void graph_free(Graph* g);
#endif 