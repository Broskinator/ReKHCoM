# Workflow: completar el arte de la vista amplia (20/10/2026)

Pedido del usuario: un solo procedimiento para generar el arte que falta fuera de
los 240x160 originales (salas, escenas, escenarios de combate y sus eventos),
**duplicando tiles del juego y dibujando pixel art solo donde sea necesario**.
Reúne lo aprendido en LOGICA_APROBADOS.md (reglas), BATTLE_SCENERY_TECHNIQUE.md
(escenarios de combate) y PRUEBAS_VISUALES.md (cómo entregar). Ante la duda,
mandan las reglas de LOGICA_APROBADOS.md.

## 1. Encontrar todas las apariciones del fondo

Un mismo fondo sale en más de un sitio: el combate, el evento que lo precede,
la derrota, la revancha. Guard Armor (20/10) tenía el patio aprobado en el
combate, pero la escena 008 (las piezas caen y se arman) usaba el mismo
fondo sin la extensión, porque esta exigía el modo combate.

- Identificar el fondo por sus datos (los mapas o tiles exactos), nunca por el
  modo, el número de combate o una bandera de estado.
- Buscar dónde se carga ese fondo (`grep` del símbolo del mapa en `src/`) y
  probar cada sitio: combate (`KHCOM_BATTLE`), evento (`KHCOM_EVENT=lista:índice`,
  ver `port/tools/test_events.py`), sala (`KHCOM_MODE=...:room...`).

## 2. Leer el arte original

1. Decodificar mapas, tiles y paletas (`KHCOM_ROOMDUMP=1` + `room_ascii.py` en
   salas; en combates, los mapas del cartucho, como `port_guard_armor.c`).
2. Ubicar el rectángulo realmente ilustrado: un mapa de 512x512 puede tener
   zonas vacías; repetir el mapa entero repite también sus huecos.
3. Clasificar cada zona:
   - **Única** (torre, escalera, marco de ventana, relieve, figura, jefe):
     nunca se copia ni se corta.
   - **Banda repetible** (fachada, tejado, arcos, pared lisa, cielo, pavimento):
     se continúa con su propio período y dirección.
   - **Borde** (festón, cornisa, bordillo): cierra el mundo donde el original
     termina en corte recto.

## 3. Generar el arte, en este orden de preferencia

1. **Duplicar tiles del juego**: repetir la banda con su período, voltear un
   tile (bits 0x400 / 0x800 de la entrada) para cerrar un borde, usar la pared
   lisa (el tile de BG3 más repetido de la columna, `MapPlainBaseTile`).
2. **Muestrear píxeles del juego** donde el tile no encaja (suelos en
   perspectiva): fila por fila, con los colores y límites de las celdas
   originales, como el pavimento de Guard Armor.
3. **Dibujar pixel art** solo si no existe tile ni píxel adecuado (autorizado
   por el usuario el 18/10):
   - mismos índices de paleta del fondo (nada de colores nuevos), mismo grosor
     de línea, mismo sombreado y misma rejilla de 8x8;
   - el dibujo continúa el del original (perspectiva, período, luz);
   - va en el repo con una nota de qué tile imita y por qué no bastaba copiar.

En los tres casos:
- Guardar **índices de paleta** y resolverlos con la paleta activa en cada
  frame: los fundidos y efectos de color del juego siguen funcionando.
- Dibujar solo fuera de los 240x160; el rectángulo GBA queda idéntico.
- No ampliar colisiones, ni cámara, ni duplicar personajes o enemigos.
- Todo bajo `PLATFORM_PC`, con una variable para apagarlo en las pruebas de
  antes/después (p. ej. `KHCOM_NO_GUARD_PATIO=1`, `KHCOM_TOPCLOSE=0`).

## 4. Probar (el usuario lo pide así)

- **Siempre en x3 entero**: `zoom=6` en `kh_com.ini` (escala 3, píxel entero).
  Una escala grande esconde los fallos de las zonas lejanas.
- Ventanas cada vez más anchas, todas en x3:
  `KHCOM_WINDOW=1920x1080`, `2560x1080`, `3440x1440` (más alto: se ve más
  cielo y suelo) y `3840x1080` (32:9).
- Varios fotogramas de cada fase: entrada, animación propia (caídas, saltos,
  ataques que mueven la cámara), fundidos y salida.
- Mismo ejecutable con la extensión activada y apagada: rectángulo GBA
  idéntico; los demás fondos, idénticos en toda la imagen.
- Revisar a ojo en las capturas anchas: costuras, escalones en diagonales,
  bandas que no casan, adornos cortados o repetidos.

## 5. Entregar y cerrar

1. Copia jugable + `Probar <tema>.bat` (PRUEBAS_VISUALES.md), con las
   capturas x3 de cada ancho y un antes/después.
2. Esperar la aprobación del usuario; no activar por defecto ni subir antes.
3. Aprobado: commit con la receta y sus límites, y una línea en
   LOGICA_APROBADOS.md.
