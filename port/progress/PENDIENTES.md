# Pendientes

Lista de trabajo pedida por el usuario, con sus ejemplos. Se actualiza en cada
commit: `[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## Reparto entre agentes (actualizar al tomar o soltar una tarea)

- **Claude**: verificación de jugabilidad (rutas reales con savestates y entradas),
  mapas de campo en 16:9 (batería de eventos `--wide`), versiones USA/JP.
- **Codex**: correcciones visuales de menús y transiciones (subida de nivel,
  Mapamundi, Naipes mundo, headers/tabs, estrés de submenús, entrada de combate).
- Si una tarea cambia de manos, moverla aquí antes de empezar.

## Pantallas y menús 16:9

- [x] Menú START: popeaba al cerrarlo y se "achicaba" al entrar en una subopción.
  Verificado fotograma a fotograma: abrir/cerrar sin saltos; al entrar/salir de una
  subopción el encuadre cambia solo en un fotograma negro de la transición.
- [x] Cristal / icono que desaparece y popea al salir del menú de guardado (y de START): el
  objeto blanco de arriba a la derecha es el icono de la sala del minimapa; el juego lo
  destruía al abrir el menú y lo recreaba al cerrarlo. Ahora se mantiene. La conservación de los demás objetos de sala sigue pendiente de revisión en las transiciones.
- [x] Subida de nivel: revisada fotograma a fotograma desde un combate de campo real (sala 10000 +
  fixture `levelup`): las barras cinemáticas (sprites del juego) cruzan todo el ancho, Sora se desliza
  sin saltos y no hay hueco ni pop; la salida vuelve a la sala limpia.
- [x] Menú de desbloquear puertas (síntesis de salas): el panel, las cartas y el nombre suben 6
  líneas sobre la barra inferior; el título, las barras y la carta del foco quedan en su sitio
  (test_priority_scenes.py --scenes doorselect, entrada y estado estable revisados).
- [x] Menú del Vórtice y "Elige un mundo": altura completa con bordes estirados (las
  capturas con barras negras eran de la build anterior; "Elige un mundo" no llenaba la altura).
- [~] Submenús en 16:9 ("todo todo"): Diario, Ver barajas, Estado, Mapamundi, Naipes:
  todos a altura completa con bordes estirados, como la pantalla de carga.
- [~] Revisar en cada pendiente si el port de Vita u otro recomp tiene una solución aplicable.
  Del port de Vita (ggdrn/KH_COM_VITA): [x] stick derecho para las cartas (L/R, arriba L+R);
  [x] devolver a la mano las cartas acumuladas (Sora): mantener V / stick derecho (pulsado
  o abajo) medio segundo; opcional en el juego y el launcher. Probado: 6 ciclos acumular/devolver
  y un truco sin crash; desactivada, la carta sigue acumulada; [ ] aviso de puntos moguri en el campo;
  [x] bancos de guardado: 5 bancos (kh_com_bank<n>.sav), opción en el juego y el launcher;
  probado: banco 1 carga la partida, banco 2 vacío ofrece solo Nueva partida y escribe su propio archivo; [ ] revancha de jefes; [ ] repetir tutoriales.
- [~] Fondos blancos fuera del mapa del castillo → negro. En el vestíbulo ya sale negro;
  si aparece blanco en otra sala, hace falta un estado allí.
- [x] Padding cian del spotlight en puerta: reproducido en la ranura 9 después
  de A. El canvas se delimita y los márgenes se dibujan negros, sin tomar el
  borde coloreado. Fixtures de spotlight y selección verifican ausencia de cian.
- [x] Barras laterales: título/menú inicial blanco, copyright crema, idioma con
  grano (sin copiar emblemas), pantalla de carga estirando el último píxel.
- [x] Arenas de combate: el escenario se repite más allá de los bordes (209 combates revisados).

## Combate

- [x] Subir el HUD de cartas (la carta sigue por debajo del borde): 8 px por
  defecto, opción "Subir cartas en combate" (0–16 px) en el juego y el launcher.
- [x] Batería de los 209 combates del menú de depuración (`port/tools/test_battles.py`): 209/209 sin cuelgues.

## Texto

- [x] Texto HD con la fuente KHMenu (cabe en los recuadros, acentos y ¿¡ nativos).
- [x] Subtítulos HD de la intro: capturas recientes EU (Comic Hearts) y JP verificadas; ambos procesos terminaron normalmente.
- [x] Recuadro de mensaje sin retrato descentrado (ranura 2 del usuario).
- [x] Texto HD en la versión JP (Yu Gothic UI sobre la rejilla del juego).
- [x] Texto dinámico de menús HD, también los números (barajas, EXP) con su color, tamaño y oclusión.

## Sonido

- [x] Música del estado 1 (Coliseo, `BGM_EVENT1`): intro → silencio → intro.
  Es así en la ROM: la última nota acaba en el tick 748 de 768 y la reverberación
  (40/127) se apaga en pocos fotogramas; el port reproduce los datos tal cual.

## Pruebas y publicación

- [x] Estados guardados deterministas (`port/tools/test_savestate.py`): guardado a mitad de combate y
  cargado en otro proceso, 1700 fotogramas idénticos píxel a píxel con pulsaciones (Sora, Riku, jefe: 3/3).

- [x] Launcher: `port/tools/test_launcher.py` instala las tres ROM y rechaza tamaño,
  SHA-1, archivo ajeno, archivo ausente y carpeta roms no escribible (11/11, avisos visibles;
  páginas en español e inglés revisadas en captura).
- [x] Audio frente a BizHawk (`port/tools/audio_compare.py`, 70 s de arranque y título US):
  correlación de muestras 0,90, de envolvente 0,92, deriva 4 ppm. Diferencias medibles:
  el port está 3,4 dB por debajo (plano de 45 Hz a 2,9 kHz) y no reproduce las imágenes
  del PCM escalonado por encima de 3 kHz (−5 a −10 dB), que el emulador sí deja pasar.
  4 minutos de título: mismas cifras. Vídeo del castillo (PCM por FIFO): correlación 0,99 y
  −3,5 dB planos en todas las bandas: la única diferencia es la ganancia general del port.
  Pendiente: canales PSG (los menús no los usan; hace falta llegar a un combate en el emulador).

- [x] Baterías: eventos US 194/194; combates US 209/209; combates como Riku 209/209;
  JP: informes terminados con 194/194 eventos y 209/209 combates sin crash.
  Son recorridos automáticos limitados, no partidas completas ni validación visual exhaustiva.
- [x] Menú de opciones del port en japonés (versión JP), con la fuente del cartucho.
- [x] Launcher: idioma del juego (EU), partidas (fecha, abrir carpeta, copia de seguridad),
  idioma del launcher, versión y firma "por MatyMeatBoy".
- [x] Carpetas limpias: build/pc sin archivos de depuración, 80 carpetas de prueba borradas, baterías
  terminadas reducidas a informe + hoja de capturas. Repo sin archivos de prueba.
- [x] Release v0.4 (pre-release pública): https://github.com/MatyMeatBoy/ReKHCom/releases/tag/v0.4
- [x] Launcher: remapeo de controles (teclado ×2 y mando por acción, guardado en kh_com.ini).
- [x] Release v0.3 (pre-release pública, no Latest) con launcher y las tres versiones:
  https://github.com/MatyMeatBoy/ReKHCom/releases/tag/v0.3
- [x] Build nueva instalada en build/pc (ini, save y estados intactos; `khcom_prev.exe` carga
  los estados viejos).
- [~] Actualizar porcentajes de PROGRESS hasta el 100 % (97 % ahora).

## Revisión de Codex — 13 de octubre de 2026

- [~] Naipes mapa, Estado/magias y barajas: los laterales ahora usan un pase
  de fondos sin sprites, para no estirar guantes, etiquetas ni cartas.
  El panel izquierdo repite cuatro píxeles del patrón entre columnas; el tab
  del ojo y la línea inferior continúan hasta el margen. Revisar todas las
  pestañas y la edición Añadir/Quitar, especialmente las transiciones.
- [~] Mapamundi: bandas ajedrezadas extendidas. Usuario confirmó el patrón;
  tramo inferior derecho ampliado y capturado. Revisar las variantes de planta.
- [~] Aviso del primer tutorial de naipes mapa: nombre de la carta oculto
  mientras el mensaje está abierto, también con fuente original. Texto HD
  comprueba oclusión en las capas de campo y menú; verificar el tutorial.
- [x] Crash de edición de baraja reproducido desde START al entrar en Añadir.
  El contador de descripción no se inicializaba; se corrigió. La ruta que fallaba
  pasa por Añadir/Quitar y vuelve al campo sin crash después de la corrección.
  La prueba nueva
  `test_menu_cycles.py` comprueba entradas a Mode_Deck y retornos a Mode_MapFld;
  doce ciclos completos pasan. La ruta anterior de veinte ciclos terminó sin
  crash, pero no validaba cada transición. Ampliar a los demás submenús y edición.
- [x] Otros crashes al alternar submenús: test_submenus.py, las seis entradas del menú
  de pausa, cuatro idas y vueltas cada una, 6/6 sin crash.
- [x] Animación superior de salida de submenús: los seis submenús revisados cada 3 fotogramas;
  la salida coincide con la GBA. Las cabeceras ya no muestran letras ocultas en el margen (US/JP).
- [x] Sprite a la izquierda después de subir de nivel: era el panel de estadísticas (BG1) que sale
  por la derecha y, al dar la vuelta el mapa de 256 px, aparecía en el margen izquierdo. PortBgClip
  deja esa capa en las columnas de la GBA hasta el final del combate.
- [x] "Elige un mundo": en la ruta natural (partida nueva → castillo → puerta, frame 8122) el texto
  y el retrato salen completos. La pérdida solo aparecía al forzar el modo sobre una sala (fixture).
- [x] Números HD: los que son texto (barajas, EXP) en HD; los de las cartas y contadores
  animados son ilustración y se conservan (HDTEXT_INVENTARIO.md).
- [x] Batería de eventos: `--wide` realmente selecciona view_mode=4. Antes
  generaba resultados etiquetados como panorámicos con view_mode=0.
- [~] Nombres de combos Sora/Riku: límite de seis entradas y gráficos del nombre
  actual corregidos bajo PLATFORM_PC. Compilan EU/US/JP; la prueba breve de un
  combate por personaje termina sin crash, falta comprobar una lista completa.
- [x] Publicaciones v0.5, v0.6 y v0.7 (pre-releases).

Validación reciente: subtítulos EU/JP en build/subtitles-current. La misma lectura
inválida de UpdateSpriteOam apareció tanto con modo forzado como entrando desde
START; la ruta de reproducción está en test_menu_cycles.py --edit.

## Cierre parcial del bloque prioritario

- [x] Cristal del castillo: objetos retenidos durante guardado se destruyen antes
  de recrearlos en salas fijas; se evita acumulación de tareas, tiles y colliders.
  Doce ciclos conservan una tarea del cristal y su dibujo antes/después.
- [~] Headers y tabs: ya no se toma el guante ni texto de la imagen final.
  Se usa una pieza del asset o tilemap original. Capturados Estado y Añadir/Quitar;
  revisar todavía Diario, Naipes mundo y las transiciones de Mapamundi.
- [x] Herramienta de regresión aislada: test_priority_scenes.py. El fixture de
  castillo abre la tarea real de guardado directamente, cuenta cristales y captura
  antes/después; no equivale a una partida ni a entradas grabadas del cartucho.
- [ ] Continuar: subida de nivel (líneas, sprite, stutter), footer de Mapamundi,
  fade pixel art de Naipes mundo, fondo de entrada de combate y estrés de submenús.
