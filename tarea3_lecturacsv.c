#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/HashMap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    char* nombre;
    int valor;
    int peso;
} Item;

typedef struct {
    int id;
    char* nombre;
    char* descripcion;
    List* items;
    int conexiones[4]; // 0: arriba, 1: abajo, 2: izquierda, 3: derecha
    int es_final;
} Escenario;

typedef struct {
    Escenario** nodos;
    int num_nodos;
} Graph;

typedef struct {
    int posicion; // ID del escenario actual
    List* inventario; // Lista de ítems recogidos
    int tiempo_restante; // Tiempo disponible
    int puntaje; // Suma de valores de ítems
    int peso_total; // Suma de pesos de ítems
    char* nombre; // Nombre del jugador
} Player;

// Funciones de creación y liberación
Graph* graph_create(int num_nodos) {
    Graph* grafo = (Graph*)malloc(sizeof(Graph));
    grafo->nodos = (Escenario**)calloc(num_nodos, sizeof(Escenario*));
    grafo->num_nodos = num_nodos;
    return grafo;
}

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

void item_free(Item* item) {
    if (item) {
        free(item->nombre);
        free(item);
    }
}

void escenario_free(Escenario* esc) {
    if (esc) {
        free(esc->nombre);
        free(esc->descripcion);
        list_clean(esc->items);
        free(esc->items);
        free(esc);
    }
}

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

Player* crear_jugador(Graph* grafo, char* nombre) {
    Player* p = (Player*)malloc(sizeof(Player));
    p->posicion = 0;
    p->inventario = list_create();
    p->tiempo_restante = 15;
    p->puntaje = 0;
    p->peso_total = 0;
    p->nombre = strdup(nombre);
    return p;
}

void player_free(Player* p) {
    if (p) {
        free(p->nombre);
        for (Item* item = list_first(p->inventario); item != NULL; item = list_next(p->inventario)) {
            item_free(item);
        }
        list_clean(p->inventario);
        free(p->inventario);
        free(p);
    }
}


// Función para leer el archivo CSV y construir el grafo
Graph* leer_escenarios_csv(const char* filename) {
    FILE* archivo = fopen(filename, "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return NULL;
    }
    char** campos = leer_linea_csv(archivo, ',');
    if (!campos) {
        fclose(archivo);
        return NULL;
    }
    free(campos);
    int num_nodos = 0;
    FILE* temp = fopen(filename, "r");
    leer_linea_csv(temp, ',');
    while (leer_linea_csv(temp, ',') != NULL) num_nodos++;
    fclose(temp);
    Graph* grafo = graph_create(num_nodos);
    archivo = fopen(filename, "r");
    leer_linea_csv(archivo, ',');
    int i = 0;
    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        int id = atoi(campos[0]);
        char* nombre = campos[1];
        char* descripcion = campos[2];
        List* items = list_create();
        List* items_str = split_string(campos[3], ";");
        for (char* item_str = list_first(items_str); item_str != NULL; item_str = list_next(items_str)) {
            List* values = split_string(item_str, ",");
            char* item_name = list_first(values);
            int item_value = atoi(list_next(values));
            int item_weight = atoi(list_next(values));
            Item* item = (Item*)malloc(sizeof(Item));
            item->nombre = strdup(item_name);
            item->valor = item_value;
            item->peso = item_weight;
            list_pushBack(items, item);
            list_clean(values);
            free(values);
        }
        list_clean(items_str);
        free(items_str);
        int arriba = atoi(campos[4]);
        int abajo = atoi(campos[5]);
        int izquierda = atoi(campos[6]);
        int derecha = atoi(campos[7]);
        int es_final = atoi(campos[8]);
        Escenario* esc = escenario_create(id, nombre, descripcion, items, arriba, abajo, izquierda, derecha, es_final);
        grafo->nodos[i++] = esc;
        free(campos);
    }
    fclose(archivo);
    return grafo;
}

// Mostrar estado actual del jugador 
void mostrar_estado(Player* p, Escenario* actual) {
    printf("\n==================== Estado Actual ====================\n");
    printf(" Jugador: %s\n", p->nombre);
    printf(" Escenario: %s (ID: %d)\n", actual->nombre, actual->id);
    printf(" Descripción: %s\n", actual->descripcion);
    printf("------------------------------------------------------\n");
    printf(" Ítems disponibles en el escenario:\n");
    if (list_size(actual->items) == 0) {
        printf("   (Ninguno)\n");
    } else {
        int i = 1;
        for (Item* item = list_first(actual->items); item != NULL; item = list_next(actual->items)) {
            printf("   %d. %s (Valor: %d pts, Peso: %d kg)\n", i++, item->nombre, item->valor, item->peso);
        }
    }
    printf("------------------------------------------------------\n");
    printf(" Inventario:\n");
    if (list_size(p->inventario) == 0) {
        printf("   (Vacío)\n");
    } else {
        for (Item* item = list_first(p->inventario); item != NULL; item = list_next(p->inventario)) {
            printf("   - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
        }
    }
    printf("------------------------------------------------------\n");
    printf(" Estadísticas:\n");
    printf("   Peso total: %d kg\n", p->peso_total);
    printf("   Puntaje: %d\n", p->puntaje);
    printf("   Tiempo restante: %d\n", p->tiempo_restante);
    printf("------------------------------------------------------\n");
    printf(" Direcciones disponibles:\n");
    if (actual->conexiones[0] != -1) printf("   1. Arriba\n");
    if (actual->conexiones[1] != -1) printf("   2. Abajo\n");
    if (actual->conexiones[2] != -1) printf("   3. Izquierda\n");
    if (actual->conexiones[3] != -1) printf("   4. Derecha\n");
    if (actual->conexiones[0] == -1 && actual->conexiones[1] == -1 &&
        actual->conexiones[2] == -1 && actual->conexiones[3] == -1) {
        printf("   (Ninguna)\n");
    }
    printf("======================================================\n");
}

void recoger_item(Player* p, Escenario* esc, Graph* grafo) {
    if (list_size(esc->items) == 0) {
        printf("No hay ítems disponibles para recoger en este escenario.\n");
        return;
    }

    printf("--- Recoger Ítems ---\n");
    printf("Selecciona el número del ítem que deseas recoger.\n");
    printf("Ingresa '0' para dejar de recoger ítems.\n");

    while (1) {
        // Mostrar los ítems disponibles en el escenario CON NÚMEROS ACTUALIZADOS
        printf("\nÍtems disponibles en %s:\n", esc->nombre);
        if (list_size(esc->items) == 0) {
            printf("  (Ya no quedan ítems aquí)\n");
            break; // Salir si no quedan ítems
        }

        int i = 1;
        Item* current_item_in_scenario = list_first(esc->items);
        while (current_item_in_scenario != NULL) {
            printf("  %d. %s (Valor: %d pts, Peso: %d kg)\n", i,
                   current_item_in_scenario->nombre,
                   current_item_in_scenario->valor,
                   current_item_in_scenario->peso);
            current_item_in_scenario = list_next(esc->items);
            i++;
        }

        printf("Tu elección: ");
        int opcion;
        scanf("%d", &opcion);
        limpiarPantalla(); // Limpia la pantalla después de cada entrada

        if (opcion == 0) {
            printf("Has terminado de recoger ítems.\n");
            break;
        }

        // Validar la opción del usuario
        if (opcion < 1 || opcion >= i) { // 'i' es el número de ítems + 1
            printf("Opción inválida. Por favor, selecciona un número de la lista.\n");
            continue;
        }

        // Buscar y "recoger" el ítem
        Item* item_a_recoger = NULL;
        int j = 1;
        current_item_in_scenario = list_first(esc->items);
        while (current_item_in_scenario != NULL) {
            if (j == opcion) {
                item_a_recoger = current_item_in_scenario;
                break;
            }
            current_item_in_scenario = list_next(esc->items);
            j++;
        }

        if (item_a_recoger != NULL) {
            list_pushBack(p->inventario, item_a_recoger); // Agrega al inventario del jugador
            p->puntaje += item_a_recoger->valor;
            p->peso_total += item_a_recoger->peso;

            // ELIMINAR el ítem del escenario
            list_remove(esc->items, item_a_recoger); // Asumiendo que list_remove maneja la eliminación del puntero de la lista sin liberar la memoria del item

            printf("¡Has recogido: %s! (Valor: %d pts, Peso: %d kg)\n",
                   item_a_recoger->nombre, item_a_recoger->valor, item_a_recoger->peso);
            printf("Tu puntaje actual: %d, Peso total: %d kg.\n", p->puntaje, p->peso_total);
        } else {
            // Esto no debería ocurrir si la validación 'opcion < 1 || opcion >= i' es correcta
            printf("Error: No se encontró el ítem seleccionado.\n");
        }
    }

    // Al finalizar la acción de recoger, se consume tiempo y se muestra el estado final.
    p->tiempo_restante -= 1;
    if (p->tiempo_restante > 0) {
        printf("\n--- Después de recoger ítems ---\n");
        mostrar_estado(p, grafo->nodos[p->posicion]); // Mostrar el estado final después de todas las recogidas
    }
}

// Descartar ítems
void descartar_item(Player* p, Escenario* esc, Graph* grafo) {
    if (list_size(p->inventario) == 0) {
        printf("El inventario está vacío.\n");
        return;
    }
    printf("Seleccione ítems a descartar (0 para terminar, número del ítem para descartar):\n");
    int i = 1;
    for (Item* item = list_first(p->inventario); item != NULL; item = list_next(p->inventario)) {
        printf("  %d. %s (Valor: %d pts, Peso: %d kg)\n", i++, item->nombre, item->valor, item->peso);
    }
    while (1) {
        int opcion;
        scanf("%d", &opcion);
        limpiarPantalla();
        if (opcion == 0) break;
        if (opcion < 1 || opcion > list_size(p->inventario)) {
            printf("Opción inválida. Intente de nuevo.\n");
            continue;
        }
        Item* item = NULL;
        i = 1;
        for (Item* it = list_first(p->inventario); it != NULL; it = list_next(p->inventario)) {
            if (i++ == opcion) {
                item = it;
                break;
            }
        }
        p->puntaje -= item->valor;
        p->peso_total -= item->peso;
        list_remove(p->inventario, item);
        printf("Ítem %s descartado.\n", item->nombre);
        item_free(item);
        mostrar_estado(p, grafo->nodos[p->posicion]); // Mostrar estado actualizado
    }
    p->tiempo_restante -= 1;
    if (p->tiempo_restante > 0) {
        mostrar_estado(p, grafo->nodos[p->posicion]); // Mostrar estado final
    }
}

// Avanzar en una dirección
void avanzar_direccion(Player* p, Escenario* actual, Graph* grafo) {
    printf("Seleccione dirección (1: Arriba, 2: Abajo, 3: Izquierda, 4: Derecha):\n");
    int dir;
    scanf("%d", &dir);
    limpiarPantalla();
    dir--;
    if (dir < 0 || dir > 3 || actual->conexiones[dir] == -1) {
        printf("Dirección inválida.\n");
        mostrar_estado(p, actual); // Mostrar estado para mantener contexto
        return;
    }
    p->posicion = actual->conexiones[dir];

    // formula pedida explicitamente, entregada en entero porque medimos el tiempo en segundos

    int tiempo_consumido = (int)ceil((p->peso_total + 1) / 10.0);
    p->tiempo_restante -= tiempo_consumido;
    printf("Avanzaste a %s. Tiempo consumido: %d.\n", grafo->nodos[p->posicion]->nombre, tiempo_consumido);
    if (grafo->nodos[p->posicion]->es_final) {
        printf("\n¡Has llegado al escenario final!\n");
        printf("Puntaje final: %d\n", p->puntaje);
        printf("Inventario final:\n");
        if (list_size(p->inventario) == 0) {
            printf("  (Vacío)\n");
        } else {
            for (Item* item = list_first(p->inventario); item != NULL; item = list_next(p->inventario)) {
                printf("  - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
            }
        }
        p->tiempo_restante = 0;
    } else if (p->tiempo_restante <= 0) {
        printf("\n¡Tiempo agotado! Has perdido.\n");
    }
    if (p->tiempo_restante > 0) {
        mostrar_estado(p, grafo->nodos[p->posicion]); // Mostrar estado tras avanzar
    }
}

void iniciar_partida(Graph* grafo, int num_jugadores) {
    // Validar número de jugadores
    if (num_jugadores != 1 && num_jugadores != 2) {
        printf("Número de jugadores no válido. Debe ser 1 o 2.\n");
        return;
    }

    // Crear jugadores
    Player* jugadores[2] = {NULL, NULL};
    char nombre[50];
    for (int i = 0; i < num_jugadores; i++) {
        printf("Ingrese el nombre del Jugador %d: ", i + 1);
        scanf("%s", nombre);
        limpiarPantalla();
        jugadores[i] = crear_jugador(grafo, nombre);
    }

    // Bucle principal del juego
    int juego_activo = 1;
    while (juego_activo) {
        int todos_final = 1;
        int todos_sin_tiempo = 1;

        // Turnos de cada jugador
        for (int i = 0; i < num_jugadores; i++) {
            Player* jugador = jugadores[i];
            if (jugador->tiempo_restante <= 0) {
                continue; // Saltar jugadores sin tiempo
            }

            printf("\n=== Turno de %s ===\n", jugador->nombre);
            Escenario* actual = grafo->nodos[jugador->posicion];
            mostrar_estado(jugador, actual);

            // Determinar número de acciones según el modo
            int max_acciones = (num_jugadores == 1) ? 1 : 2;
            int acciones = 0;
            while (acciones < max_acciones && jugador->tiempo_restante > 0) {
                if (num_jugadores == 1) {
                    printf("\n--- Opciones ---\n");
                    printf("1. Recoger ítem\n");
                    printf("2. Descartar ítem\n");
                    printf("3. Avanzar dirección\n");
                    printf("4. Reiniciar\n");
                    printf("5. Salir\n");
                    printf("Seleccione: ");
                } else {
                    printf("\nAcción %d (1: Recoger ítem, 2: Descartar ítem, 3: Avanzar, 4: Pasar): ", acciones + 1);
                }

                int op;
                scanf("%d", &op);
                limpiarPantalla();

                if (num_jugadores == 1) {
                    switch (op) {
                        case 1:
                            recoger_item(jugador, actual, grafo);
                            break;
                        case 2:
                            descartar_item(jugador, actual, grafo);
                            break;
                        case 3:
                            avanzar_direccion(jugador, actual, grafo);
                            break;
                        case 4:
                            player_free(jugador);
                            jugadores[i] = crear_jugador(grafo, jugador->nombre);
                            printf("Partida reiniciada.\n");
                            break;
                        case 5:
                            printf("Saliendo de la partida.\n");
                            for (int j = 0; j < num_jugadores; j++) {
                                if (jugadores[j]) player_free(jugadores[j]);
                            }
                            return;
                        default:
                            printf("Opción inválida.\n");
                            mostrar_estado(jugador, actual);
                    }
                } else {
                    if (op == 4) break;
                    switch (op) {
                        case 1:
                            recoger_item(jugador, actual, grafo);
                            acciones++;
                            break;
                        case 2:
                            descartar_item(jugador, actual, grafo);
                            acciones++;
                            break;
                        case 3:
                            avanzar_direccion(jugador, actual, grafo);
                            acciones++;
                            break;
                        default:
                            printf("Opción inválida.\n");
                            mostrar_estado(jugador, actual);
                    }
                }

                if (grafo->nodos[jugador->posicion]->es_final) break;
            }
        }

        // Mostrar resumen de los jugadores (solo en modo multijugador)
        if (num_jugadores == 2) {
            printf("\n=== Resumen del Turno ===\n");
            for (int i = 0; i < num_jugadores; i++) {
                Player* jugador = jugadores[i];
                printf("%s: Posición ID %d, Puntaje %d, Peso %d kg, Tiempo %d\n",
                       jugador->nombre, jugador->posicion, jugador->puntaje, jugador->peso_total, jugador->tiempo_restante);
            }
            printf("=========================\n");
        }

        // Verificar condiciones de finalización
        for (int i = 0; i < num_jugadores; i++) {
            Player* jugador = jugadores[i];
            if (!grafo->nodos[jugador->posicion]->es_final) {
                todos_final = 0;
            }
            if (jugador->tiempo_restante > 0) {
                todos_sin_tiempo = 0;
            }
        }

        if (todos_final || todos_sin_tiempo) {
            if (todos_final) {
                int puntaje_total = 0;
                for (int i = 0; i < num_jugadores; i++) {
                    puntaje_total += jugadores[i]->puntaje;
                }
                if (num_jugadores == 1) {
                    printf("\n¡Has llegado al escenario final!\n");
                    printf("Puntaje final: %d\n", puntaje_total);
                } else {
                    printf("\n¡Ambos jugadores han llegado a la salida!\n");
                    printf("Puntaje total: %d\n", puntaje_total);
                }
                for (int i = 0; i < num_jugadores; i++) {
                    Player* jugador = jugadores[i];
                    printf("Inventario de %s:\n", jugador->nombre);
                    if (list_size(jugador->inventario) == 0) {
                        printf("  (Vacío)\n");
                    } else {
                        for (Item* item = list_first(jugador->inventario); item != NULL; item = list_next(jugador->inventario)) {
                            printf("  - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
                        }
                    }
                }
            } else if (num_jugadores == 1) {
                printf("\n¡Tiempo agotado! Has perdido.\n");
            } else {
                printf("\n¡Tiempo agotado para ambos jugadores! Han perdido.\n");
            }
            juego_activo = 0;
        }
    }

    // Liberar memoria de los jugadores
    for (int i = 0; i < num_jugadores; i++) {
        if (jugadores[i]) player_free(jugadores[i]);
    }
}

// no especifican que si un usuario no puede robarle el item XD

int main() {
    Graph* grafo = NULL;
    while (1) {
        printf("\n=== MENÚ PRINCIPAL ===\n");
        printf("1. Cargar Laberinto desde Archivo CSV\n");
        printf("2. Iniciar Partida\n");
        printf("3. Salir\n");
        printf("Seleccione: ");
        int opcion;
        scanf("%d", &opcion);
        limpiarPantalla();
        if (opcion == 1) {
            if (grafo) graph_free(grafo);
            grafo = leer_escenarios_csv("data/graphquest.csv");
            if (grafo) {
                printf("Laberinto cargado con éxito.\n");
            } else {
                printf("Error al cargar el laberinto.\n");
            }
        } else if (opcion == 2) {
            if (!grafo) {
                printf("Primero debes cargar el laberinto.\n");
                continue;
            }
            printf("¿Cuántos jugadores? (1 o 2): ");
            int jugadores;
            scanf("%d", &jugadores);
            limpiarPantalla();
            iniciar_partida(grafo, jugadores);
        } else if (opcion == 3) {
            printf("¡Hasta luego!\n");
            if (grafo) graph_free(grafo);
            break;
        } else {
            printf("Opción inválida.\n");
        }
    }
    return 0;
}