# 074 Monstruo: preliminar aceptada

**Actualización: mejora aprobada por el usuario.** Receta vigente:
`py -3 port/tools/scene074_polish_preview.py`. Header de integración actualizado.
La forma izquierda es una copia completa del mismo motivo que aparece cortado
a la derecha: 19 entradas originales ya visibles coinciden con desplazamiento
(+20,-1) tiles. Para completar la cola y eliminar el fragmento repetido, BG2
destino x32..34, y17..21 usa fuente (x-20,y+1). Cambian 507 píxeles del añadido,
sin alterar la parte original del encuadre. Esta regla se documentó en el punto12
de REVISION_TILES_ESCENAS.md para reutilizar el método, no sus coordenadas.

Usuario aprueba el layout y la corrección de paleta activa; pide guardar esta
base antes de intentar reparar la pieza verde del suelo abajo a la derecha,
usando como referencia la pieza dibujada abajo a la izquierda.

Receta: `py -3 port/tools/scene074_tile_preview.py`.
Captura original: `build/scene-review/run1/0_074/before/shot_00100.bmp`.
Paleta activa: snapshot privado `build/scene-review/palette074/frame99.khst`.
Base congelada: `build/scene-review/accepted074/`; incluye manifest SHA256.

Mapa declarado 512x512; bloque dibujado 256x256. Las tres capas BG3/BG2/BG1
se extienden a derecha y abajo. Dentro del bloque original se conserva todo,
incluido el vacío de arriba/izquierda. La vista actual solo muestra el cruce
del borde derecho; el borde inferior requiere otra vista antes de validarse.

Para main: `port/include/port/port_scene074_tiles.h` reproduce las coordenadas
fuente. Llamar solo para evento074, también dentro de los bloques default de
la declaración 512x512. Leer tile original directamente y procesar con la
paleta y efectos activos del PPU. No convertir esta paleta temporal en asset
estático: el efecto de aclarado cambia durante la escena. Los registros de
blending están desactivados en esta captura; el efecto está en la paleta viva.

Todavía no hay hook nuevo en `port_ppu.c`: main está integrando ese archivo.
La sustitución inferior aprobada sí forma parte del perfil vigente; el commit
preliminar `8cf93bc78` conserva la versión anterior para comparación.
