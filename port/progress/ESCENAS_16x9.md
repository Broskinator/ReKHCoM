# Escenas en 16:9 — extensión del fondo con tiles originales

Revisión con el usuario, escena a escena, **de 2 en 2**. Nada entra en la lista
`kIsoScenes` (port/src/port_ppu.c) sin su aprobación. Aprobada: 004 (Leon).

## Qué escenas tocar

- Solo las que se salen de un mapa cuyo escenario llega al borde y se corta en
  seco (callejón de Leon, playa, sala de cristal, suelo de Monstruo que "se
  iba a ver"): ahí se continúa el escenario.
- Las que tienen un **marco diseñado** (sala que flota en el vacío, como la de
  Naminé en el castillo) se dejan como están: ese borde está pensado para verse.
- Dentro de un mismo mapa puede haber las dos cosas (Monstruo: el vacío de
  arriba/izquierda queda negro, el suelo de abajo/derecha se continúa). Por eso
  cada escena lista **sus bordes a mano** (`PORT_ISO_LEFT/RIGHT/TOP/BOTTOM`).
- 44 escenas en 33 fondos cruzan el borde del mapa en 16:9
  (build/scene-review/run1/report.json). Muchas comparten fondo.

## Cómo se continúa (lógica validada por el usuario)

1. **Seguir la diagonal isométrica del juego**, no copiar en vertical ni en
   horizontal: las líneas de la escena (base de las paredes, bordes, cornisas)
   tienen que seguir su trazo fuera del mapa.
2. **Usar el período real del mapa**: el paso diagonal con el que el propio
   mapa se repite (2x1 tiles en Leon, 4x2 en el cristal, 8x4 en la playa),
   buscado entre 2k x k para k = 1..8 en ambas diagonales.
3. **La dirección la deciden las líneas, no el relleno**: al puntuar, cada
   coincidencia pesa 1/frecuencia del tile; el suelo repetido casi no cuenta y
   los tiles de pared/borde mandan (si no, gana la diagonal equivocada).
4. **Capas**: en algunos mapas las paredes están en BG2 (cristal), en otros en
   BG3 con objetos sueltos en BG2 (cajas de Leon). `PORT_ISO_UPPER` extiende
   también las capas de encima; los tiles únicos (cajas, manchas) se saltan
   siguiendo la diagonal, así no se duplican objetos.
5. Esquinas: si la diagonal de un borde vuelve a sacar el punto por el lado ya
   cruzado, se usa la diagonal espejo del mismo período.
6. Si aun así no hay tile del que copiar, queda negro (nunca se inventa).

## Herramientas

- `build/scene-review/tools/scenes.py EXE DIR --only l:i,...`: antes/después
  ocultos (KHCOM_NOISO / lista o KHCOM_ISOALL con SCENES_ALL=1).
- `build/scene-review/tools/scene_card.py`: tarjeta con mapa completo y
  cuadros de cámara (cian GBA, rojo 16:9).
- `KHCOM_ISOLOG=1`: el juego registra los pasos elegidos por capa.

## Estado

- 061 (cristal, comparte fondo con 192): bordes L/R/T/B + capas de encima.
  Muy mejorada; falta la esquina superior derecha (tiles "pelones").
- 057 (playa): con solo suelo quedan huecos y escalones en la pared; con capas
  de encima peor. Necesita trabajo fino.
- 074 (Monstruo): pendiente; solo abajo y derecha (el usuario lo marcó).
