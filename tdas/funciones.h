# include "list.h"

typedef struct {
    int posicion; // ID del escenario actual
    List* inventario; // Lista de ítems recogidos (usando list.h)
    int tiempo_restante; // Tiempo disponible
    int puntaje; // Suma de valores de ítems
    int peso_total; // Suma de pesos de ítems
    char* nombre; // Nombre del jugador
} Player;