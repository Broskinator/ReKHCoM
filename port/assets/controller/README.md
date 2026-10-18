# Ultimate 2C: arte elegido por el usuario

El usuario proporcionó `Gemini_Generated_Image_lukhx5lukhx5lukh.jpg` y pidió usar
ese dibujo, quitar el fondo y añadir las funciones de remapeo del menú nativo.
La foto `8bitdo.webp` fue una referencia anterior; el arte activo deriva del JPG
de Gemini elegido después. La interpretación anterior no se usa.

- `ultimate2c-transparent.png`: edición con la herramienta integrada imagegen
  para retirar fondo blanco y hueco central, preservando dibujo/colores/botones.
- `ultimate2c-sprite.png`: representación nativa de 104x72, 48 colores y cobertura
  binaria; generada por el importador a partir de la edición transparente.
- `port/src/port_controller_art.inc`: los mismos índices/paleta embebidos. No se
  requiere decodificador de imágenes ni archivos externos al ejecutar el juego.

Regenerar los dos artefactos nativos: `python port/tools/import_controller_art.py`.
El importador solo hace normalización para el framebuffer (recorte de cobertura,
muestreo nearest y paleta sin dithering). La eliminación del fondo se hizo con
imagegen, no mediante una herramienta local de edición.

## Prompt de edición utilizado (herramienta integrada, sin CLI/API externa)

Edit target: this exact provided Gemini pixel-art controller sprite. User has
selected THIS version for their game. Remove the flat white background to true
transparent alpha, including all white inside the wide U-shaped space between
the grips. Preserve the controller pixels, silhouette, peach and cream palette,
shading, all buttons and their positions, pixel edges, A/B/X/Y letters, central
heart symbol, and overall proportions as faithfully as possible. Do NOT redesign,
do NOT add controls, labels, shadows, halo, glow, props or text. Clean any JPEG
white fringe just outside the dark outline while retaining the outline itself.
The result is one isolated pixel-art game controller sprite, centered with very
little transparent padding. Crisp square pixels with hard alpha edge, no smoothing
and no blurred outline. Transparent background, no white plate, no drawn checkerboard.
This is background removal of the user's chosen art, not a new interpretation.
