# v1.8 — especificación y regresiones

## Opciones

Pedido final: eliminar las dos filas «Solo escalas enteras» y «Escalado».
Todas las escalas actuales siguen disponibles. x6 lleva «íntegra x1» y aviso;
x3, «íntegra x2». Son las referencias de interfaz pedidas por el usuario y no
cambian la geometría ni renumeran los demás pasos. No mostrar procedencia v1.4.
Default explícito x4 (ID 8), también en 32:9; config v10 migra el preset 0 a x4.
Las selecciones explícitas se conservan. Un antiguo medio paso redondeado por
integer_zoom=1 se convierte a su valor efectivo antes de retirar el filtro.

## Transparencia EXP

Pedido final: no aplicar texto HD a los números de EXP; usar el arte original
incluso con HD activado. Diálogos, subtítulos y menús conservan el HD.
El usuario aclaró que la posición anterior estaba bien. Se descartó el intento
de anclar el aviso al HUD. src/btl/btl_exp.c conserva exactamente su posición y suprime el tag de texto
únicamente durante el dibujo de sus números, restaurando el tag del llamador.
El defecto confirmado: al sustituir tinta 14/15 por números HD, el PPU omitía
esos píxeles y dejaba transparente su antigua silueta. Se conserva además la corrección genérica: pinta el color 13
del panel antes de sustituir tinta en celdas style 2, en mundo y HUD. Los
avisos de EXP ya no pasan por esa sustitución. Se conserva el contorno originalmente transparente y los fades.
No se alteran experiencia, niveles ni tiempos de los avisos.

`python port/tools/test_exp_panel.py`: 768 comprobaciones de píxel, sin fallos,
HD activado/desactivado, mundo/HUD, normal/iluminado/oscurecido. Fixture ROM-free.
`python port/tools/test_exp_hud.py`: tarea real del juego en todos los zooms de
16:9 y 32:9, EXP ganada/siguiente nivel/subida; además x4 con texto HD y tamaños
HUD. Ver informe local build/v18-review/exp/report.json al terminar.
La acción textlevel del harness cambia solo la memoria visual de la tarea para
que su actualización original genere el aviso; no otorga niveles ni EXP reales.

## Cinemáticas: estados proporcionados

Se copiaron las ranuras 1/2/3 de la carpeta local v1.7.1. Se cargan correctamente
con su ejecutable compatible; los originales no se escriben.
- Ranura 1: Event, conversación inicial de Ciudad de Paso, negro a la izquierda.
- Ranura 2: Event, mensaje de Llave prístina, sprites sobre fondo negro.
- Ranura 3: Mode_MapFix, mismo defecto de límite de mapa.

Fuera de los límites de los bloques, RenderBg devolvía transparente y la
composición elegía negro. Ahora puede continuar el color de una esquina del
mapa solamente si su tile es completamente uniforme y opaco. Nunca se repite
arte de escenario ni UI. El color pasa por la paleta y efectos del mismo BG.

Reproducción de memoria de dibujo de las tres ranuras: 480×270 y 1280×360.
En 1/3 el borde pasa de negro al marrón de la escena (RGB 24,16,0). En las seis
comparaciones, los 240×160 píxeles de la imagen original son idénticos antes y
después. Informe: build/v172-review/cutscene-padding-report.json.
`python port/tools/test_world_padding.py`: 810 píxeles exteriores comprobados;
la base falla en 270 y la corrección no falla. Las esquinas con patrón o
transparentes conservan el comportamiento anterior.

La reproducción dibujada no restaura punteros de código ni ejecuta el juego:
world_padding_conformance.c lee exclusivamente regiones GBA y estado PPU,
valida la firma específica de la fixture v1.7.1 EU y descarta referencias
nativas. No implementar una omisión de la firma en el cargador del jugador.
No subir estados/ROM/dumps al repositorio ni al ZIP.

La ranura 2 aislada ya muestra negro correcto al cargar desde cero; los bordes
blancos reportados dependen de la transición previa. Para mensajes de solo
sprites, Present rellena ahora todos los márgenes con el fondo renderizado
actual, sin depender del estilo anterior; el muestreo incluye fades. El panel
y sus letras se conservan. Los bordes de películas con BG activo no cambian.

## Entrega

Publicar v1.8 pública con launcher, EU/US/JP y SHA-256. No modificar tags ni
partidas anteriores. La carpeta local nueva recibe copias de .sav y ajustes;
los quick states de v1.7.1 se conservan con su ejecutable compatible.

La propuesta v1.7.2 no se publicó: el usuario pidió pasar directamente a v1.8.
La equivalencia del aviso original con HD activado/desactivado se comprueba
píxel por píxel en sus tres estados, a x4 en 16:9 y 32:9.

Oracle PPU final: 104/107, idéntico conjunto de fallos históricos HD HUD
(cover_bg/cover_obj/window) de la referencia. Todas las pruebas de píxeles
de hardware pasan; no hay fallos nuevos. Informe local:
build/ppu-conformance/20261016_005012_896889/report.json.

## Cierre de verificación

- 32 configuraciones pasan; tres estados de aviso cada una. Las seis
  comparaciones de recorte EXP con HD on/off (tres estados, dos resoluciones)
  son idénticas. Informe build/v18-review/exp/report.json.
- EU/US/JP compiladas y ejecutadas con la fixture de EXP. Guardar/cargar estado
  nuevo pasa; informe build/v18-review/state y regional-report.json.
- ZIP v1.8: 21.069.645 bytes; SHA-256
  8eba27896dc31b840accdfc03b1d4b76beadac7de5fc3adb2013174f84c285c8.
  Los cuatro ejecutables del ZIP coinciden con los artefactos compilados.
- Carpeta local y Probar ReKHCoM v1.8.bat preparados. Guardados normales y
  configuración copiados sin sobrescribir los originales. Estados antiguos
  permanecen en v1.7.1. ZIP público sin ROM, .sav, .khst ni configuración local.

## Publicación confirmada

https://github.com/MatyMeatBoy/ReKHCom/releases/tag/v1.8
Release 414522263, pública y latest, 16/10/2026 03:58:46 UTC.
Tag sobre 7efad75d5. Digest y tamaños de ambos assets calculados por GitHub
coinciden con los archivos locales. main y codex/v1.5-fixes pusheados.
No se publicó v1.7.2 ni se modificaron tags anteriores.
