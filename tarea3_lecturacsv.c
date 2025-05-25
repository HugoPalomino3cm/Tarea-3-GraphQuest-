#include "tdas/extra.h"
#include "tdas/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Definiciones de colores ANSI
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"

typedef struct {
    int id;
    char nombre[100];
    char descripcion[300];
    List* items; // Lista de ítems (cada ítem puede ser struct o string parseado)
    int arriba, abajo, izquierda, derecha; // Conexiones
    int esFinal;
} Escenario;

typedef struct {
    char nombre[50];
    int valor;
    int peso;
} Item;

/**
 * Carga canciones desde un archivo CSV
 */
List* leer_escenarios() {
    List* escenarios = list_create();

    FILE *archivo = fopen("data/graphquest.csv", "r");
    if (archivo == NULL) {
        perror("Error al abrir el archivo");
        return escenarios;
    }

    char **campos;
    leer_linea_csv(archivo, ','); // encabezado

    while ((campos = leer_linea_csv(archivo, ',')) != NULL) {
        Escenario *e = malloc(sizeof(Escenario));
        e->id = atoi(campos[0]);
        strcpy(e->nombre, campos[1]);
        strcpy(e->descripcion, campos[2]);

        e->items = list_create();
        List *itemsStrings = split_string(campos[3], ";");
        for (char *item_str = list_first(itemsStrings); item_str != NULL; item_str = list_next(itemsStrings)) 
        {
            List *values = split_string(item_str, ",");
            Item *item = malloc(sizeof(Item));
            strcpy(item->nombre, (char*)list_get(values, 0));
            item->valor = atoi((char*)list_get(values, 1));
            item->peso = atoi((char*)list_get(values, 2));
            list_pushBack(e->items, item);
            list_clean(values);
            free(values);
        }
        list_clean(itemsStrings);
        free(itemsStrings);

        e->arriba = atoi(campos[4]);
        e->abajo = atoi(campos[5]);
        e->izquierda = atoi(campos[6]);
        e->derecha = atoi(campos[7]);
        e->esFinal = atoi(campos[8]);

        list_pushBack(escenarios, e);
    }

    fclose(archivo);
    return escenarios;
}

typedef struct NodoGrafo {
    Escenario* escenario;
    struct NodoGrafo* arriba;
    struct NodoGrafo* abajo;
    struct NodoGrafo* izquierda;
    struct NodoGrafo* derecha;
} NodoGrafo;

NodoGrafo* buscar_nodo_por_id(List* nodos, int id) {
    for (NodoGrafo* nodo = list_first(nodos); nodo != NULL; nodo = list_next(nodos)) {
        if (nodo->escenario->id == id)
            return nodo;
    }
    return NULL;
}

List* construir_grafo(List* escenarios) {
    List* nodos = list_create();

    // Paso 1: crear nodos del grafo
    for (Escenario* esc = list_first(escenarios); esc != NULL; esc = list_next(escenarios)) {
        NodoGrafo* nodo = malloc(sizeof(NodoGrafo));
        nodo->escenario = esc;
        nodo->arriba = nodo->abajo = nodo->izquierda = nodo->derecha = NULL;
        list_pushBack(nodos, nodo);
    }

    // Paso 2: enlazar nodos según sus conexiones
    for (NodoGrafo* nodo = list_first(nodos); nodo != NULL; nodo = list_next(nodos)) {
        Escenario* esc = nodo->escenario;
        nodo->arriba = buscar_nodo_por_id(nodos, esc->arriba);
        nodo->abajo = buscar_nodo_por_id(nodos, esc->abajo);
        nodo->izquierda = buscar_nodo_por_id(nodos, esc->izquierda);
        nodo->derecha = buscar_nodo_por_id(nodos, esc->derecha);
    }

    return nodos; // lista de nodos conectados
}

// Menú principal con diseño más estético
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

void play(List* escenarios, int num_jugadores) {
    Escenario *escenarioPorId[100] = {NULL};
    for (Escenario* e = list_first(escenarios); e != NULL; e = list_next(escenarios)) {
        if (e->id >= 0 && e->id < 100) {
            escenarioPorId[e->id] = e;
        }
    }

    // Estado para hasta 2 jugadores
    Escenario *escenario_actual[2] = {escenarioPorId[1], escenarioPorId[1]};
    List *inventario[2] = {list_create(), list_create()};
    int tiempo[2] = {10, 10};

    int turno = 0;

    while (1) {
        limpiarPantalla();

        // Referencias del jugador actual
        Escenario *escenario = escenario_actual[turno];
        List *inv = inventario[turno];
        int *t = &tiempo[turno];

        // Mostrar estado del juego de forma más clara
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
            for (Item *item = list_first(escenario->items); item != NULL; item = list_next(escenario->items)) {
                printf("  %d) %s (Valor: %d pts, Peso: %d kg)\n", i++, item->nombre, item->valor, item->peso);
            }
        }
        printf("------------------------\n");

        printf(YELLOW "INVENTARIO:\n" RESET);
        int peso_total = 0, puntaje_total = 0;
        if (list_size(inv) == 0) {
            printf("  (Tu inventario está vacío)\n");
        } else {
            for (Item *item = list_first(inv); item != NULL; item = list_next(inv)) {
                printf("  - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
                puntaje_total += item->valor;
                peso_total += item->peso;
            }
        }
        printf("  Peso total: %d kg | Puntaje total: %d pts\n", peso_total, puntaje_total);
        printf("------------------------\n");

        printf(YELLOW "TIEMPO RESTANTE: %d segundos\n" RESET, *t);
        printf("------------------------\n");

        // Mostrar opciones de movimiento de forma intuitiva
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

        int acciones = 0;
        while (acciones < 2) {
            char opcion;
            scanf(" %c", &opcion);

            if (opcion == '0') {
                printf("Turno terminado anticipadamente.\n");
                break;
            }

            if (opcion == '1') {
                if (list_size(escenario->items) == 0) {
                    printf("No hay ítems para recoger.\n");
                } else {
                    printf("Elige un ítem para recoger (0 para cancelar):\n");
                    int i = 1;
                    for (Item *item = list_first(escenario->items); item != NULL; item = list_next(escenario->items)) {
                        printf("  %d) %s\n", i++, item->nombre);
                    }
                    int elem;
                    scanf("%d", &elem);
                    if (elem > 0 && elem <= list_size(escenario->items)) {
                        Item *item = list_remove_at(escenario->items, elem - 1);
                        list_pushBack(inv, item);
                        printf(GREEN "¡Has recogido %s!\n" RESET, item->nombre);
                        presioneTeclaParaContinuar();
                    }
                }
                (*t) -= 1;
            } else if (opcion == '2') {
                if (list_size(inv) == 0) {
                    printf("No hay ítems en tu inventario.\n");
                } else {
                    printf("Elige un ítem para descartar (0 para cancelar):\n");
                    int i = 1;
                    for (Item *item = list_first(inv); item != NULL; item = list_next(inv)) {
                        printf("  %d) %s\n", i++, item->nombre);
                    }
                    int elem;
                    scanf("%d", &elem);
                    if (elem > 0 && elem <= list_size(inv)) {
                        Item *item = list_remove_at(inv, elem - 1);
                        free(item);
                        printf(GREEN "¡Has descartado %s!\n" RESET, item->nombre);
                    }
                }
                (*t) -= 1;
            } else if (opcion == '3') {
                // Reiniciar partida: reinicia tiempo, inventarios y posición de ambos jugadores
                for (int i = 0; i < num_jugadores; i++) {
                    tiempo[i] = 10;
                    list_clean(inventario[i]);
                    escenario_actual[i] = escenarioPorId[1];
                }
                printf(GREEN "¡Partida reiniciada con éxito!\n" RESET);
                presioneTeclaParaContinuar();
                break; // salir del turno actual
            } else if (opcion == '4') {
                printf(CYAN "Saliendo de la partida...\n" RESET);
                for (int i = 0; i < num_jugadores; i++) {
                    list_clean(inventario[i]);
                    free(inventario[i]);
                }
                presioneTeclaParaContinuar();
                return;
            } else if (opcion == 'w') {
                if (escenario->arriba != -1) escenario = escenarioPorId[escenario->arriba];
                (*t) -= (peso_total + 1) / 10;
            } else if (opcion == 's') {
                if (escenario->abajo != -1) escenario = escenarioPorId[escenario->abajo];
                (*t) -= (peso_total + 1) / 10;
            } else if (opcion == 'a') {
                if (escenario->izquierda != -1) escenario = escenarioPorId[escenario->izquierda];
                (*t) -= (peso_total + 1) / 10;
            } else if (opcion == 'd') {
                if (escenario->derecha != -1) escenario = escenarioPorId[escenario->derecha];
                (*t) -= (peso_total + 1) / 10;
            } else {
                printf("Opción no válida. Intenta de nuevo.\n");
                continue;
            }

            // Actualiza la posición del jugador actual
            escenario_actual[turno] = escenario;

            acciones++;
            limpiarPantalla();
            // Volver a mostrar el estado tras cada acción
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
                for (Item *item = list_first(escenario->items); item != NULL; item = list_next(escenario->items)) {
                    printf("  %d) %s (Valor: %d pts, Peso: %d kg)\n", i++, item->nombre, item->valor, item->peso);
                }
            }
            printf("------------------------\n");
            printf(YELLOW "INVENTARIO:\n" RESET);
            peso_total = 0, puntaje_total = 0;
            if (list_size(inv) == 0) {
                printf("  (Tu inventario está vacío)\n");
            } else {
                for (Item *item = list_first(inv); item != NULL; item = list_next(inv)) {
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

        // Verificar condiciones de fin de juego
        int jugadores_terminaron = 0, jugadores_sin_tiempo = 0;
        for (int i = 0; i < num_jugadores; i++) {
            if (escenario_actual[i]->esFinal) jugadores_terminaron++;
            if (tiempo[i] <= 0) jugadores_sin_tiempo++;
        }

        if (jugadores_terminaron == num_jugadores || jugadores_sin_tiempo == num_jugadores) {
            printf("\n");
            printf(RED "✦══════ ¡FIN DEL JUEGO! ══════✦\n" RESET);
            printf("------------------------\n");
            for (int i = 0; i < num_jugadores; i++) {
                printf(YELLOW "INVENTARIO DEL JUGADOR %d:\n" RESET, i + 1);
                int total = 0;
                for (Item *item = list_first(inventario[i]); item != NULL; item = list_next(inventario[i])) {
                    printf("  - %s (Valor: %d pts, Peso: %d kg)\n", item->nombre, item->valor, item->peso);
                    total += item->valor;
                }
                printf("  Puntaje del Jugador %d: %d pts\n", i + 1, total);
            }
            printf("------------------------\n");
            presioneTeclaParaContinuar();
            return;
        }

        presioneTeclaParaContinuar();

        // Cambiar turno si hay 2 jugadores
        if (num_jugadores == 2) turno = (turno + 1) % 2;
    }
}

void printdelosjugadores(List *escenarios)
{
    printf(MAGENTA "✦══════ SELECCIÓN DE MODO DE JUEGO ══════✦\n" RESET);
    printf("------------------------\n");
    printf("Elige cómo quieres jugar:\n");
    printf(GREEN "  ✧ [1] Modo Individual (1 jugador)\n" RESET);
    printf(GREEN "  ✧ [2] Modo Multijugador (2 jugadores)\n" RESET);
    printf("------------------------\n");
    printf(CYAN "Ingresa tu elección (1 o 2): " RESET);
    int modo;
    scanf("%d", &modo);

    if (modo == 1) {
        play(escenarios, 1);
    } else if (modo == 2) {
        play(escenarios, 2);
    } else {
        printf("Opción no válida. Intenta de nuevo.\n");
        presioneTeclaParaContinuar();
    }
}

int main() {
    char opcion;

    do {
        mostrarMenuPrincipal();
        scanf(" %c", &opcion);

        limpiarPantalla();

        switch (opcion) {
        case '1': {
            List* escenarios = leer_escenarios();
            if (escenarios == NULL || list_size(escenarios) == 0) {
                printf("No se pudieron cargar los escenarios.\n");
                presioneTeclaParaContinuar();
                break;
            }
            printdelosjugadores(escenarios);
            // Liberar memoria al salir de play (ya se maneja internamente)
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