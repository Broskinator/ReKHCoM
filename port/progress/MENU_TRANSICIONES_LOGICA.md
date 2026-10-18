# Opciones: transición y luz del cursor — 18/10/2026

Encargo directo del usuario: evitar que el juego se reduzca al abrir Opciones,
conservar el fundido y tamaños del menú, y usar la luz animada del debug original.

## Implementación

- PortUiActive sigue siendo propietario de entrada/pausa durante toda la apertura.
  PortUiMenuCanvas solo cambia lienzo/presentación cuando los ocho frames de
  salida de la escena ya llegaron a negro. El presentador toma la fase antes de
  que PortUiDraw avance el reloj; la última escena no usa por accidente la fase
  siguiente. Al cerrar, el último menú llega a negro antes de restaurar el juego.
- AutoView usa la fase visual para reservar el lienzo Enhanced del menú. El ajuste
  de escenas GBA (sFit) solo se aplica fuera de ese lienzo. Así no reduce el
  menú ni desactiva prematuramente la presentación ampliada del juego.
- PortUiSceneFade oscurece el resultado final del renderer, incluyendo HUD,
  texto HD y demás capas, en vez de oscurecer únicamente el framebuffer del mundo.
  No se necesita capturar la pantalla ni guardar una textura por cada frame.
- MenuLight lee gDebugMenuCursorAnim0, sus frames, tiles y paleta de la ROM.
  La secuencia original es 0,1,2,1,0,3, ocho ticks por paso. Copia solo la cara
  luminosa 5x5 dentro del cuadro existente; barras, cuadros y texto conservan sus
  dimensiones. La selección nueva reinicia el ciclo. El reloj normal es tiempo
  SDL a 60 ticks/s; KHCOM_MENU_FRAMECLOCK permite grabación reproducible del harness.

## Evidencia y límites

- EU/US/JP compilan; las tres regiones dibujan sus menús con los recursos propios.
- Capturas de salida reales 1920x1080 en build/menu-transition-review:
  title, gba (idiomas), field y controls-motion. Los vídeos menu.mp4
  muestran entrada, navegación y regreso al juego. Estas pruebas usan SDL software.
- Comparación de escena antes del menú contra dos frames de salida: error medio
  residual por canal tras compensar solo el brillo <0.31/255 en campo congelado y
  <0.11/255 en idiomas. Las coordenadas se conservan; no se compensó escala/posición.
- La luz observada recorre RGB (255,231,99), (255,140,49), (255,16,0),
  (156,16,0) en los distintos pasos. El vídeo mueve el foco A→B→L y el dibujo
  resalta A→B→LB, sin remapear esas acciones.
- test_zoom.py: 56 casos pasan. Se actualizó su stub al nombre de la nueva
  consulta visual; la pausa/propiedad de entrada sigue usando PortUiActive.
- No se afirma validación de todas las escenas, GPU físicas o plataformas a partir
  de estas capturas. La aprobación artística del nuevo mando es independiente.
