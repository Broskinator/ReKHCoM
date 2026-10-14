# Verificación de mapas y arenas — 14/10/2026

## Arenas: inventario y cambio

`test_wide_world_assets.py` lee únicamente las ROM locales y YAML de assets;
decodifica las 19 arenas affine de 512×512 (incluye Hades, definido en
event_backgrounds.yaml). Las 19 imágenes de índices de color coinciden entre
EU/US/JP; también coinciden las 19 imágenes RGB completas, incluidas paletas.
No se distribuyen los PNG extraídos ni las ROM.

Revisión individual y hojas de 6 arenas al tamaño nativo: el canvas original
contiene 48 píxeles de padding a izquierda/derecha, excepto Hook (24).
Recircular los 512 píxeles repite ese padding. Recircular toda la imagen
también puede repetir elementos únicos en pantallas muy anchas.

`src/btl/btl_map.c` ahora asigna bandas de tiles originales por arena. El
compositor recibe límites, origen y ancho de cada banda. Las bandas tienen
64–96 píxeles; dejan fuera el corazón/portón de Garden, árbol de Wonderland,
luna de Halloween, calle de Twilight, portón de Mansion y mástil de Hook.
Hook toma su banda derecha de la barandilla izquierda para conservar un solo
mástil. El muestreo reflejado del compositor evita una unión abrupta al
comenzar la banda. OBJ y colisiones no participan en esa extensión.

Evidencia local reproducible: `build/codex-wide-assets/{eu,us,jp}/inventory.json`,
`arenas_0..3.png`, `bands_review0..3.png` y `{asset}_bands.png`. El inventario
marca explícitamente revisión requerida; las medidas de costura no son un PASS.

Runner serial `test_wide_world_battles.py`, build debug aislada
`build/codex-wide-eu`: 19/19 fixtures sin crash y 57 capturas a 1280×720 en
`build/codex-wide-battles/20261014_040226_841136`. Revisados individualmente
los frames380/600 de las 19 arenas y 180 de Marluxia/Hum. Las bandas eliminan
padding lateral y conservan motivos centrales únicos. Se probaron además
abismos de Atlantica/Wonderland/Monstro y fondos de duelos con efectos.

Observación a contrastar: Hook frame380, el trazo rosa del efecto llega al
retrato del HUD en margen izquierdo. No atribuirlo a las bandas: estas solo
muestrean BG, y la revisión debe comparar build anterior y compositor actual.

Pendiente: comparación del canvas GBA, revisión regional, rotación/zoom y
salida. La build US y JP aisladas se están preparando. La batería inicial EU
precede el guard extra de compositor charbase0/screenbase12; repetir casos de
efectos con el guard actual antes de cerrar la barra.

## Mapas: primera pasada visual

Fuente: `build/codex-gameplay-rooms/20261014_034832_461231`, 13 capturas
individuales a 1280×720, fixture de MAP del agente gameplay.

Revisados frames 260,560,860,1160,1460,1760,2060,2360,2660,2960 y3560:
bordes exteriores, abismos y sus colores de fondo continuos, sin estirar
tiles, duplicar objetos ni rellenar con suelo inexistente.

Los frames3260 y3860 no son pruebas válidas de mundos finales: el primero
selecciona TwilightTown en planta1 (el mundo corresponde a planta10); el
segundo es FORMAT, un escenario de depuración. El inventario MAP no contiene
100AcreWood. Este hallazgo se comunicó al agente gameplay para corregir casos.

`MapWorldTile` reproduce los mismos tiles/fallbacks que `MapBuildBgColumn`
y `MapBuildBgRow`, incluyendo transparencia BG3 y tiles exteriores BG2/BG1.
Los índices negativos se descartan en `MapGetCell`; no se encontró un acceso
negativo válido que justificara cambiar el código.

Regresión independiente `test_wide_world_cells.py`: PASS, 417216 comparaciones
del callback con los cuerpos originales de `MapBuildBgRow`/`MapBuildBgColumn`.
Se compilan los cuerpos exactos de producción; el fixture contiene 20 celdas,
BG1/BG2 ausentes, BG3 presente, coordenadas negativas y fuera de los cuatro
bordes. Informe: `build/codex-wide-cells/report.txt`.

Esta pasada no cierra field maps: faltan variantes, huecos internos, salas
fijas, movimiento de cámara, TwilightTown válido y las regiones US/JP.
