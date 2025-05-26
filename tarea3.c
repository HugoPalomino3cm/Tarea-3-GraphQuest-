#include "tdas/extra.h"
#include "tdas/list.h"
#include "tdas/Hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

// Definiciones de colores ANSI
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

//Escenario para representar un Escenario dentro del juego
typedef struct {
    int id;
    char nombre[100];
    char descripcion[300];
    List* items; // Lista de ítems (cada ítem es un struct Item)
    int arriba, abajo, izquierda, derecha; // Conexiones
    int esFinal;
} Escenario;

//Estructura para representar un item del juego
typedef struct {
    char nombre[50];
    int valor;
    int peso;
} Item; 

//Estructura para representar un nodo del grafo
typedef struct NodoGrafo {
    Escenario* escenario;
    struct NodoGrafo* arriba;
    struct NodoGrafo* abajo;
    struct NodoGrafo* izquierda;
    struct NodoGrafo* derecha;
} NodoGrafo;

/**
 * Convierte el valor del campo EsFinal ("Si" o "No") en un valor booleano.
 * @param valor Cadena que contiene "Si" o "No".
 * @return true si el valor es "Si", false si es "No", false en caso de valor inválido.
 */
bool es_final(const char* valor) {
    if (strcmp(valor, "Si") == 0) {
        return true;
    } else if (strcmp(valor, "No") == 0) {
        return false;
    }
    return false; // Valor por defecto para casos inválidos
}

/**
 * Carga escenarios desde un archivo CSV y los almacena en una lista.
 * @return Lista de escenarios o NULL si hay un error.
 */
List* leer_escenarios() {
    List* escenarios = list_create();
    if (!escenarios) {
        perror("ERROR AL CREAR LISTA DE ESCENARIOS\n");
        return NULL;
    }

    FILE *archivo = fopen("data/graphquest.csv", "r");
    if (!archivo) {
        perror("Error al abrir el archivo");
        list_clean(escenarios);
        free(escenarios);
        return NULL;
    }

    char **campos;
    leer_linea_csv(archivo, ','); // Ignora el encabezado

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        Escenario *e = malloc(sizeof(Escenario));
        if (!e) {
            perror("Error al asignar memoria para Escenario\n");
            free(campos);
            continue;
        }

        e->id = atoi(campos[0]);
        strncpy(e->nombre, campos[1], sizeof(e->nombre) - 1);
        e->nombre[sizeof(e->nombre) - 1] = '\0';
        strncpy(e->descripcion, campos[2], sizeof(e->descripcion) - 1);
        e->descripcion[sizeof(e->descripcion) - 1] = '\0';

        e->items = list_create();
        if (!e->items) {
            perror("Error al crear lista de ítems\n");
            free(e);
            free(campos);
            continue;
        }
        List *itemsStrings = split_string(campos[3], ";");
        if (itemsStrings) {
            for (char *item_str = list_first(itemsStrings); item_str != NULL; item_str = list_next(itemsStrings)) {
                List *values = split_string(item_str, ",");
                if (values) {
                    Item *item = malloc(sizeof(Item));
                    if (!item) {
                        perror("Error al asignar memoria para ítem\n");
                        list_clean(values);
                        free(values);
                        continue;
                    }
                    strncpy(item->nombre, (char*)list_get(values, 0), sizeof(item->nombre) - 1);
                    item->nombre[sizeof(item->nombre) - 1] = '\0';
                    item->valor = atoi((char*)list_get(values, 1));
                    item->peso = atoi((char*)list_get(values, 2));
                    list_pushBack(e->items, item);
                    list_clean(values);
                    free(values);
                }
            }
            list_clean(itemsStrings);
            free(itemsStrings);
        }

        e->arriba = atoi(campos[4]);
        e->abajo = atoi(campos[5]);
        e->izquierda = atoi(campos[6]);
        e->derecha = atoi(campos[7]);
        e->esFinal = es_final(campos[8]);

        list_pushBack(escenarios, e);
        free(campos);
    }

    fclose(archivo);
    return escenarios;
}

/**
 * Construye un grafo a partir de una lista de escenarios, conectando nodos según sus IDs.
 * @param escenarios Lista de escenarios.
 * @return HashMap con nodos conectados o NULL si existe un error.
 */
HashMap* construir_grafo(List* escenarios) {
    if (!escenarios) return NULL;

    HashMap* nodos = createMap(list_size(escenarios) * 2);
    if (!nodos) {
        perror("Error al crear HashMap\n");
        return NULL;
    }

    // Crear nodos del grafo
    for (Escenario* esc = list_first(escenarios); esc != NULL; esc = list_next(escenarios)) {
        NodoGrafo* nodo = malloc(sizeof(NodoGrafo));
        if (!nodo) {
            perror("Error al asignar memoria para NodoGrafo\n");
            hashmap_clean(nodos);
            free(nodos);
            return NULL;
        }
        nodo->escenario = esc;
        nodo->arriba = nodo->abajo = nodo->izquierda = nodo->derecha = NULL;
        insertMap(nodos, esc->id, nodo);
    }

    // Enlazar nodos según conexiones
    for (Pair* pair = firstMap(nodos); pair != NULL; pair = nextMap(nodos)) {
        NodoGrafo* nodo = (NodoGrafo*)pair->value;
        Escenario* esc = nodo->escenario;

        if (esc->arriba != -1) {
            Pair* x = searchMap(nodos, esc->arriba);
            nodo->arriba = x ? (NodoGrafo*)x->value : NULL;
        }
        if (esc->abajo != -1) {
            Pair* x = searchMap(nodos, esc->abajo);
            nodo->abajo = x ? (NodoGrafo*)x->value : NULL;
        }
        if (esc->izquierda != -1) {
            Pair* x = searchMap(nodos, esc->izquierda);
            nodo->izquierda = x ? (NodoGrafo*)x->value : NULL;
        }
        if (esc->derecha != -1) {
            Pair* x = searchMap(nodos, esc->derecha);
            nodo->derecha = x ? (NodoGrafo*)x->value : NULL;
        }
    }

    return nodos;
}

/**
 * Libera los recursos del juego (escenarios e ítems).
 * @param escenarios Lista de escenarios totales.
 */
void liberar_escenarios(List* escenarios) {
    if (!escenarios) return;
    for (Escenario* e = list_first(escenarios); e != NULL; e = list_next(escenarios)) {
        if (e->items) {
            for (Item* item = list_first(e->items); item != NULL; item = list_next(e->items)) {
                free(item);
            }
            list_clean(e->items);
            free(e->items);
        }
        free(e);
    }
    list_clean(escenarios);
    free(escenarios);
}

/**
 * Libera los nodos del grafo.
 * @param nodos HashMap con los nodos del grafo principal del juego.
 */
void liberar_nodos(HashMap* nodos) {
    if (!nodos) return;
    for (Pair* pair = firstMap(nodos); pair != NULL; pair = nextMap(nodos)) {
        free(pair->value);
    }
    hashmap_clean(nodos);
    free(nodos);
}

/**
 * Muestra el menú principal del juego con opciones de jugar o salir.
 */
void mostrarMenuPrincipal() {
    limpiarPantalla();
    printf(RED "✦══════════════════════════════════════════════════════✦\n" RESET);
    printf(RED "                  BIENVENIDO A GRAPHQUEST               \n" RESET);
    printf(RED "✦══════════════════════════════════════════════════════✦\n" RESET);
    printf("\n");
    printf(GREEN "  ✧ [1] JUGAR\n" RESET);
    printf(GREEN "  ✧ [2] SALIR\n" RESET);
    printf("\n");
    printf(CYAN "✦═══════════════════════✦\n" RESET);
    printf(CYAN "Ingresa tu opción (1 o 2): " RESET);
}

/**
 * Muestra la interfaz del turno del jugador.
 * @param escenario Puntero al escenario actual.
 * @param inv Lista de ítems en el inventario del jugador.
 * @param t Puntero al tiempo restante del jugador o los jugadores.
 * @param turno Índice del jugador actual (0 o 1).
 * @param num_jugadores Número de jugadores (1 o 2).
 */
void mostrar_interfaz_turno(Escenario* escenario, List* inv, int* t, int turno, int num_jugadores) {
    printf(RED "✦═══ TURNO DEL JUGADOR %d ═══✦\n" RESET, turno + 1);
    printf("------------------------\n");
    printf(YELLOW "UBICACIÓN ACTUAL:\n" RESET);
    printf("  Escenario: %s\n", escenario->nombre);
    printf("  Descripción: %s\n", escenario->descripcion);
    printf("------------------------\n");

    printf(GREEN "ÍTEMS DISPONIBLES EN EL ESCENARIO:\n" RESET);
    if (list_size(escenario->items) == 0) {
        printf("  (No hay ítems disponibles)\n");
    } else {
        int i = 1;
        for (Item* item = list_first(escenario->items); item != NULL; item = list_next(escenario->items)) {
            printf("  %d) %s (Valor: %d pts, Peso: %d kg)\n", i++, item->nombre, item->valor, item->peso);
        }
    }
    printf("------------------------\n");

    printf(YELLOW "INVENTARIO:\n" RESET);
    int peso_total = 0, puntaje_total = 0;
    if (list_size(inv) == 0) {
        printf("  (Tu inventario está vacío)\n");
    } else {
        for (Item* item = list_first(inv); item != NULL; item = list_next(inv)) {
            printf("  - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
            puntaje_total += item->valor;
            peso_total += item->peso;
        }
    }
    printf("  Peso total: %d kg | Puntaje total: %d pts\n", peso_total, puntaje_total);
    printf("------------------------\n");

    printf(YELLOW "TIEMPO RESTANTE: %d segundos\n" RESET, *t);
    printf("------------------------\n");

    printf(BLUE "MOVIMIENTOS DISPONIBLES:\n" RESET);
    if (escenario->arriba != -1) printf("  [w] Moverse hacia ARRIBA\n");
    if (escenario->abajo != -1) printf("  [s] Moverse hacia ABAJO\n");
    if (escenario->izquierda != -1) printf("  [a] Moverse a la IZQUIERDA\n");
    if (escenario->derecha != -1) printf("  [d] Moverse a la DERECHA\n");
    if (escenario->arriba == -1 && escenario->abajo == -1 && escenario->izquierda == -1 && escenario->derecha == -1) {
        printf("  (No hay movimientos disponibles)\n");
    }
    printf("------------------------\n");

    printf(MAGENTA "ACCIONES DISPONIBLES:\n" RESET);
    printf("  [1] Recoger un ítem del escenario\n");
    printf("  [2] Descartar un ítem de tu inventario\n");
    printf("  [3] Reiniciar la partida\n");
    printf("  [4] Salir de la partida\n");
    printf("------------------------\n");

    printf(CYAN "INSTRUCCIONES:\n" RESET);
    printf("  - Elige una acción o movimiento (1-4 o w/a/s/d).\n");
    printf("  - Puedes realizar hasta %d acciones por turno.\n", num_jugadores == 1 ? 1 : 2);
    printf("  - Escribe [0] para terminar el turno antes de tiempo.\n");
    printf(">> ");
}

/**
 * Muestra el mensaje de fin de juego y los resultados de los jugadores.
 * @param escenario_actual Arreglo de nodos de escenario actual para cada jugador.
 * @param inventario Arreglo de listas de inventario para cada jugador.
 * @param tiempo Arreglo de tiempos restantes para cada jugador.
 * @param num_jugadores Número de jugadores (1 o 2).
 */
void mostrar_fin_juego(NodoGrafo* escenario_actual[], List* inventario[], int tiempo[], int num_jugadores) {
    printf("\n" RED "✦══════ ¡FIN DEL JUEGO! ══════✦\n" RESET);
    printf("------------------------\n");
    for (int i = 0; i < num_jugadores; i++) {
        if (escenario_actual[i]->escenario->esFinal) {
            printf(GREEN "¡FELICITACIONES JUGADOR %d! HAS LLEGADO A LA SALIDA.\n" RESET, i + 1);
            printf("------------------------\n");
        }
        if (tiempo[i] <= 0) {
            printf(RED "¡JUGADOR %d HA PERDIDO! SE ACABÓ EL TIEMPO.\n" RESET, i + 1);
            printf("------------------------\n");
        }
        printf(YELLOW "INVENTARIO DEL JUGADOR %d:\n" RESET, i + 1);
        int total = 0;
        if (list_size(inventario[i]) == 0) {
            printf("  (Inventario vacío)\n");
        } else {
            for (Item* item = list_first(inventario[i]); item != NULL; item = list_next(inventario[i])) {
                printf("  - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
                total += item->valor;
            }
        }
        printf("  Puntaje final: %d pts\n", total);
        printf("------------------------\n");
    }
}

/**
 * Ejecuta el juego, manejando turnos, movimientos y acciones de los jugadores.
 * @param nodos HashMap con el grafo de nodos.
 * @param num_jugadores Número de jugadores (1 o 2).
 */
void play(HashMap* nodos, int num_jugadores) {
    if (!nodos) {
        printf("Error: Grafo no inicializado.\n");
        return;
    }

    Pair* start = searchMap(nodos, 1);
    NodoGrafo* nodo_inicial = start ? (NodoGrafo*)start->value : NULL;
    if (!nodo_inicial) {
        printf("Error: No se encontró el escenario inicial (ID 1).\n");
        return;
    }

    NodoGrafo* escenario_actual[2] = {nodo_inicial, nodo_inicial};
    List* inventario[2] = {list_create(), list_create()};
    int tiempo[2] = {10, 10};

    // Verificar creación de inventarios
    for (int i = 0; i < 2; i++) {
        if (!inventario[i]) {
            printf("Error: No se pudo crear el inventario del jugador %d.\n", i + 1);
            for (int j = 0; j < i; j++) {
                list_clean(inventario[j]);
                free(inventario[j]);
            }
            return;
        }
    }

    int turno = 0;

    while (1) {
        // Verificar condición de fin de juego al inicio del turno
        int jugadores_terminaron = 0, jugadores_sin_tiempo = 0;
        for (int i = 0; i < num_jugadores; i++) {
            if (escenario_actual[i]->escenario->esFinal) jugadores_terminaron++;
            if (tiempo[i] <= 0) jugadores_sin_tiempo++;
        }

        if (jugadores_terminaron == num_jugadores || jugadores_sin_tiempo == num_jugadores) {
            limpiarPantalla();
            mostrar_fin_juego(escenario_actual, inventario, tiempo, num_jugadores);
            presioneTeclaParaContinuar();
            // Liberar inventarios
            for (int i = 0; i < 2; i++) {
                for (Item* item = list_first(inventario[i]); item != NULL; item = list_next(inventario[i])) {
                    free(item);
                }
                list_clean(inventario[i]);
                free(inventario[i]);
            }
            return;
        }

        limpiarPantalla();

        NodoGrafo* nodo_actual = escenario_actual[turno];
        Escenario* escenario = nodo_actual->escenario;
        List* inv = inventario[turno];
        int* t = &tiempo[turno];
        int peso_total = 0;

        // Calcular peso total del inventario
        for (Item* item = list_first(inv); item != NULL; item = list_next(inv)) {
            peso_total += item->peso;
        }

        mostrar_interfaz_turno(escenario, inv, t, turno, num_jugadores);

        int acciones = 0;
        while (acciones < (num_jugadores == 1 ? 1 : 2)) {
            char opcion;
            scanf(" %c", &opcion);
            while (getchar() != '\n'); // Limpiar buffer

            if (opcion == '0') break;

            if (opcion == '1') {
                if (list_size(escenario->items) == 0) {
                    printf("No hay ítems para recoger.\n");
                } else {
                    printf("Elige un ítem para recoger (0 para cancelar):\n");
                    int i = 1;
                    for (Item* item = list_first(escenario->items); item != NULL; item = list_next(escenario->items)) {
                        printf("  %d) %s\n", i++, item->nombre);
                    }
                    int elem;
                    char input[100];
                    if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%d", &elem) != 1 || elem < 0 || elem > list_size(escenario->items)) {
                        printf("Entrada inválida.\n");
                        continue;
                    }
                    if (elem > 0) {
                        Item* item = list_remove_at(escenario->items, elem - 1);
                        list_pushBack(inv, item);
                        printf(GREEN "¡Has recogido %s!\n" RESET, item->nombre);
                        presioneTeclaParaContinuar();
                    }
                }
                (*t)--;
            } else if (opcion == '2') {
                if (list_size(inv) == 0) {
                    printf("No hay ítems en tu inventario.\n");
                } else {
                    printf("Elige un ítem para descartar (0 para cancelar):\n");
                    int i = 1;
                    for (Item* item = list_first(inv); item != NULL; item = list_next(inv)) {
                        printf("  %d) %s\n", i++, item->nombre);
                    }
                    int elem;
                    char input[100];
                    if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%d", &elem) != 1 || elem < 0 || elem > list_size(inv)) {
                        printf("Entrada inválida.\n");
                        continue;
                    }
                    if (elem > 0) {
                        Item* item = list_remove_at(inv, elem - 1);
                        printf(GREEN "¡Has descartado %s!\n" RESET, item->nombre);
                        free(item);
                        presioneTeclaParaContinuar();
                    }
                }
                (*t)--;
            } else if (opcion == '3') {
                for (int i = 0; i < num_jugadores; i++) {
                    tiempo[i] = 10;
                    for (Item* item = list_first(inventario[i]); item != NULL; item = list_next(inventario[i])) {
                        free(item);
                    }
                    list_clean(inventario[i]);
                    escenario_actual[i] = nodo_inicial;
                }
                printf(GREEN "¡Partida reiniciada con éxito!\n" RESET);
                presioneTeclaParaContinuar();
                break;
            } else if (opcion == '4') {
                printf(CYAN "Saliendo de la partida...\n" RESET);
                for (int i = 0; i < 2; i++) {
                    for (Item* item = list_first(inventario[i]); item != NULL; item = list_next(inventario[i])) {
                        free(item);
                    }
                    list_clean(inventario[i]);
                    free(inventario[i]);
                }
                presioneTeclaParaContinuar();
                return;
            } else if ((opcion == 'w' && escenario->arriba != -1) ||
                       (opcion == 's' && escenario->abajo != -1) ||
                       (opcion == 'a' && escenario->izquierda != -1) ||
                       (opcion == 'd' && escenario->derecha != -1)) {
                int next_id = (opcion == 'w') ? escenario->arriba :
                              (opcion == 's') ? escenario->abajo :
                              (opcion == 'a') ? escenario->izquierda :
                              escenario->derecha;
                Pair* p = searchMap(nodos, next_id);
                if (p) {
                    escenario_actual[turno] = (NodoGrafo*)p->value;
                    (*t) -= (int)ceil((double)(peso_total + 1) / 10.0);
                    printf(GREEN "¡Te has movido hacia %s!\n" RESET,
                           opcion == 'w' ? "arriba" :
                           opcion == 's' ? "abajo" :
                           opcion == 'a' ? "la izquierda" : "la derecha");
                    presioneTeclaParaContinuar();
                } else {
                    printf("Movimiento inválido.\n");
                    continue;
                }
            } else {
                printf("Opción no válida. Intenta de nuevo.\n");
                continue;
            }

            escenario = escenario_actual[turno]->escenario;
            acciones++;
            if (acciones < (num_jugadores == 1 ? 1 : 2)) {
                limpiarPantalla();
                // Recalcular peso total después de posibles cambios en el inventario
                peso_total = 0;
                for (Item* item = list_first(inv); item != NULL; item = list_next(inv)) {
                    peso_total += item->peso;
                }
                mostrar_interfaz_turno(escenario, inv, t, turno, num_jugadores);
            }
        }

        turno = (turno + 1) % num_jugadores;
    }
}

/**
 * Permite al usuario seleccionar el modo de juego (individual o multijugador) y ejecuta la partida.
 * @param escenarios Lista de escenarios cargados.
 */
void seleccionar_modo_juego(List *escenarios) {
    printf(MAGENTA "✦══════ SELECCIÓN DE MODO DE JUEGO ══════✦\n" RESET);
    printf("------------------------\n");
    printf("Elige cómo quieres jugar:\n");
    printf(GREEN "  ✧ [1] Modo Individual (1 jugador)\n" RESET);
    printf(GREEN "  ✧ [2] Modo Multijugador (2 jugadores)\n" RESET);
    printf("------------------------\n");
    printf(CYAN "Ingresa tu elección (1 o 2): " RESET);
    int modo;
    char input[100];
    if (!fgets(input, sizeof(input), stdin) || sscanf(input, "%d", &modo) != 1 || (modo != 1 && modo != 2)) {
        printf("Opción no válida. Intenta de nuevo.\n");
        presioneTeclaParaContinuar();
        return;
    }

    HashMap* nodos = construir_grafo(escenarios);
    if (!nodos) {
        printf("Error al construir grafo\n");
        liberar_escenarios(escenarios);
        presioneTeclaParaContinuar();
        return;
    }

    if (modo == 1) {
        play(nodos, 1);
    } else {
        play(nodos, 2);
    }

    liberar_nodos(nodos);
    liberar_escenarios(escenarios);
    presioneTeclaParaContinuar();
}

/**
 * Función principal que inicia el juego y maneja el menú principal.
 */
int main() {
    char opcion;

    do {
        mostrarMenuPrincipal();
        scanf(" %c", &opcion);
        while (getchar() != '\n'); // Limpiar buffer

        limpiarPantalla();

        switch (opcion) {
        case '1': {
            List* escenarios = leer_escenarios();
            if (!escenarios || list_size(escenarios) == 0) {
                printf("No se pudieron cargar los escenarios.\n");
                if (escenarios) {
                    liberar_escenarios(escenarios);
                }
                presioneTeclaParaContinuar();
                break;
            }
            seleccionar_modo_juego(escenarios);
            break;
        }
        case '2':
            printf(RED "✦═══════════════════════════════════════✦\n" RESET);
            printf(CYAN "¡GRACIAS POR JUGAR GRAPHQUEST! SALIENDO...\n" RESET);
            printf(RED "✦═══════════════════════════════════════✦\n" RESET);
            break;
        default:
            printf("Opción no válida. Por favor, intenta de nuevo.\n");
            printf(RED "✦═══════════════════════✦\n" RESET);
            break;
        }
        presioneTeclaParaContinuar();
    } while (opcion != '2');

    return 0;
}