<div align="center">

<h1 style="font-size: 3em;">🎮 GraphQuest </h1>

<div style="background-color: #f9f9f9; border: 2px solid #e0e0e0; border-radius: 15px; padding: 20px; max-width: 600px; box-shadow: 0 4px 8px rgba(0,0,0,0.1);">

<img src="https://ik.imagekit.io/storybird/images/eed760b4-0d8a-4806-a9b1-a36875e617e5/0_155117436.webp?tr=q-80" width="300" style="border-radius: 10px;"/>

<p style="font-size: 1.2em; margin-top: 15px;">
<strong>Explora escenarios, recoge objetos y sobrevive por turnos en este juego de estrategia.</strong>
</p>

</div>

---

## 👥 Integrantes

| ![Felipe Romero](https://github.com/HugoPalomino3cm/Tarea-2-Spotifind-/blob/3ad963c848e352f40585c1d90a62faa69bf326c7/image.png)| ![Hugo Palomino](https://github.com/HugoPalomino3cm/Tarea-2-Spotifind-/blob/d48628f1a78cbea3c8fe0ca7ac8cb5dbf44ae92a/fotoMia.png) |
|:--:|:--:|
| **Felipe Romero** | **Hugo Palomino** |

---

## 🛠️ Requisitos Previos

- [Git](https://git-scm.com/) instalado.
- Compilador **GCC**.
- **PowerShell** (recomendado para Windows).
- Sistema operativo: Windows, Linux o macOS.
- [Visual Studio Code](https://code.visualstudio.com/) (recomendado).

---

![GIF Externo](https://cdn.hashnode.com/res/hashnode/image/upload/v1666975601963/U7VvHXeDV.gif)

---

## 📦 Clonar y Compilar el Proyecto

### 1. Clonar el Repositorio

Abre **Visual Studio Code** y sigue estos pasos:

- Abre la pestaña de **Control de Código Fuente** (ícono de Git).
- Haz clic en **"Clonar repositorio"** e ingresa la siguiente URL:

```bash
https://github.com/HugoPalomino3cm/Tarea-3-GraphQuest-.git
```

- Elige una carpeta local para guardar el proyecto y ábrela en VSCode.

---

### 2. Compilar el Proyecto

Abre una terminal en VSCode (`Terminal -> New Terminal`) o usa PowerShell y ejecuta:

```bash
gcc tdas/*.c tarea3.c -Wno-unused-result -o tarea3
```

> ⚠️ Si estás usando PowerShell, añade esto para que se muestren bien los caracteres especiales:

```powershell
$OutputEncoding = [Console]::OutputEncoding = [Text.UTF8Encoding]::new()
```

---

### 3. Ejecutar el Programa

```bash
./tarea3
```

---

## 💡 Ejemplo de Uso

A continuación se muestra un ejemplo de cómo interactuar con **GraphQuest**:

---

### 📌 Cómo jugar GraphQuest

1. **Inicia el programa.**  
   Se te mostrará un menú con dos opciones: Para comenzar una partida, **debes presionar la tecla `1` y luego Enter**. Esto iniciará el juego y cargará el mapa de escenarios.

2. **Selecciona la opción 1 para comenzar a jugar.**  
El juego cargará el mapa de escenarios y te pedirá que selecciones el número de jugadores:

3. **Empieza la partida.**  
Verás la descripción del escenario donde te encuentras, los ítems disponibles, tu inventario y el tiempo restante.

4. **Durante tu turno puedes:**
- Moverte entre escenarios usando las teclas:
  - `w` → Arriba  
  - `a` → Izquierda  
  - `s` → Abajo  
  - `d` → Derecha  
- Recoger un ítem del escenario con la opción `1`
- Descartar un ítem de tu inventario con la opción `2`
- Reiniciar la partida con `3`
- Salir del juego con `4`
- Escribir `0` para terminar tu turno antes de tiempo

5. **Cada acción consume tiempo.**  
- Moverte te consume más tiempo si llevas muchos objetos.  
- Cuando se acaba el tiempo de un jugador, termina su participación.

6. **El objetivo del juego:**  
Recolectar los ítems más valiosos posibles antes de que se acabe el tiempo, gestionando el peso que cargas y explorando los distintos escenarios conectados.

7. **Repite turnos** entre jugadores hasta que decidan salir o se acabe el tiempo.

---

## ⚠️ Posibles Errores Comunes

- falta poner los errores

![GIF Externo](https://media.tenor.com/X8854xxuQ_EAAAAM/destroy-code-mad.gif)

# Contribuciones

## Aportes individuales

### Hugo Palomino
-falta

### Felipe Romero
-falta

### Trabajo en conjunto
-falta

