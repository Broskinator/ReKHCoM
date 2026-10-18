# Lógica de los arreglos aprobados por el usuario (vista amplia)

Reglas que se repiten en todo lo que el usuario ha aprobado. Aplicarlas antes
de proponer algo nuevo; ante un caso nuevo, derivar la solución de ellas.

## Principios

1. **Solo píxeles del juego.** Nada se dibuja nuevo: se reutilizan los tiles de
   la propia sala/escena, volteados o repetidos (bits 0x400 hflip / 0x800 vflip
   de la entrada). Si de verdad no hay tile adecuado, se puede dibujar uno, con
   el mismo estilo (el usuario lo autorizó el 18/10).
2. **Un borde natural cierra el mundo.** Donde la sala termina en corte recto
   se continúa con el *propio borde* de la sala (su festón inferior) volteado,
   vacío fuera y muro dentro. Nunca un corte recto ni un zigzag inventado.
3. **Nunca repetir adornos.** Marcos de ventana, relieves y figuras no se
   copian ni se cortan por la mitad: el espacio nuevo se rellena con la pared
   lisa de esa columna.
4. **Pared lisa = capa base.** En estas salas BG3 lleva la pared de rayas y
   BG2 solo los adornos; fuera de la sala: BG2 vacío (0) y BG3 = el tile de
   BG3 más repetido de esa columna (`MapPlainBaseTile`). Elegir por frecuencia
   dentro de la columna, no "el primer tile".
5. **Detección genérica, no por número de sala.** Buscar en los datos de la
   sala la tira que cumple una forma (24 tiles seguidos con borde arriba y
   vacío justo debajo) en vez de fijar coordenadas; así sirve a otras salas
   del mismo tileset. Si no se encuentra, no se toca nada (comportamiento
   original).
6. **GBA intacta.** Todo vive bajo `PLATFORM_PC` y solo fuera de la ventana
   original; se puede apagar (`KHCOM_TOPCLOSE=0`).

## Método (rápido)

1. Reproducir la sala exacta y capturar (ver PRUEBAS_VISUALES.md).
2. Volcar los tiles: `KHCOM_ROOMDUMP=1` → `roomdump.txt` (BG1/2/3) y
   `python port/tools/room_ascii.py roomdump.txt --bg 1 --rows .. --cols ..`
   para ver la forma del borde, las tiras repetibles (`--strip`) y qué tiles
   son comunes.
3. Probar la idea con variables de entorno antes de fijarla; recortar y ampliar
   la zona; comparar con lo que marcó el usuario (azul = quitar, naranja = usar).
4. Entregar copia jugable + .bat; esperar aprobación; solo entonces dejarlo
   activo por defecto y anotarlo aquí.

## Errores que el usuario descartó

- Continuar el muro hacia arriba repitiendo la fila superior (eco de textura).
- Zigzag con las escaleras de las esquinas (piezas que no encajan).
- Elegir como "pared lisa" una celda con marco de ventana: se ve media ventana
  flotando. La pared lisa se decide por tile y por frecuencia, en BG3.

## Aprobados

- 17: borde superior de la sala de Riku (Hollow Bastion B1, sala 3): aprobado
  totalmente el 18/10 (`src/map/map_cell.c`: `MapFindEdgeStrip`,
  `MapTopEdgeTile`, `MapPlainBaseTile`).
- 061 hielo y 074 Monstro: aprobadas (18/10). 057 playa: aprobada con mejoras
  (igualar la estructura de tiles del lado derecho-superior con la del izquierdo;
  no cortar motivos de pasto por la mitad).
- Anteriores (márgenes de ventanas 1-17, escenas 004/057/061/074, Guard Armor):
  ver REPARTO.md y ESCENAS_REVISION_AMPLIA.md; mismos principios 1-3.

- 59 retratos: aprobado el 18/10. La vista amplia exponía las posiciones de espera
  X=449/-299 antes de aplicar la silueta. Ocultar la espera en `msgface_2` y
  recortar su entrada/salida al área original de la ventana desplazada. Aplicar
  en la función común, sin excepciones por personaje/escena. Commit `acd85c818`;
  detalle en RETRATOS_59_LOGICA.md.
