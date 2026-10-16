# v1.8.1 — Estado y selector

El usuario informa que el resto de v1.8 funciona bien. Se limita la corrección
al margen de Estado y se quitan los pasos intermedios por pedido adicional.

## Causa de las franjas

SideBar, estilo MENU, detectaba una diferencia entre el píxel compuesto del
borde y el fondo durante PortMenuSliding. Extendía el píxel compuesto a todo
el margen. Los avisos NUEVO y los textos también podían ocupar ese píxel:
las franjas blancas/rojas eran sus letras ampliadas horizontalmente.

Estado tiene paneles de BG propios. PortMenuBackgroundOnly consulta el modo
actual y evita allí la regla de cubrir el fondo con la columna de sprites.
Se mantienen el patrón del panel, pestañas y banda de cabecera. Los otros
menús conservan su tratamiento anterior. Se deriva del modo en cada dibujo,
por lo que cargar un estado no requiere restaurar una bandera extra.

## Selector

Solo pasos numerados de píxel; se retiran los medios pasos, sin reintroducir
las filas de escalado/enteras. x4 sigue predeterminado, referencias x6→íntegra
x1 con aviso y x3→íntegra x2 conservadas. Config v11 redondea una selección
intermedia antigua hacia el siguiente entero. Las selecciones enteras no cambian.
`python port/tools/test_zoom.py`: 41 combinaciones de resolución/escala,
selector sin medios pasos, migración y x4 en 16:9/32:9.

## Regresión de Estado

`python port/tools/test_status_margins.py --reference <build-base>` usa la tarea
real de Estado: habilidades aprendidas, avisos NUEVO on/off, entrada y salida.
La base contiene la misma acción statusnew del harness; el cambio de render
se compara con variables idénticas. La acción solo opera en KHCOM_MODE y no
modifica partidas del usuario. No se suben ROMs ni capturas privadas.

Se desactivan los backends de mandos físicos solamente en los subprocesos de
prueba: el usuario puede seguir jugando en otra ventana sin dirigir las
capturas automatizadas. No se cambia la entrada del juego publicado.
Informes locales: build/status-transition/review-isolated.

El oráculo compara márgenes con NUEVO on/off (deben ser idénticos tras el fix)
y el rectángulo original contra la referencia (debe permanecer idéntico).
Se cubren entrada, pantalla estable y salida, tanto 16:9 como 32:9.
