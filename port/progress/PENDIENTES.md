# Pendientes

Lista de trabajo pedida por el usuario, con sus ejemplos. Se actualiza en cada
commit: `[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## Pantallas y menús 16:9

- [x] Menú START: popeaba al cerrarlo y se "achicaba" al entrar en una subopción.
  Verificado fotograma a fotograma: abrir/cerrar sin saltos; al entrar/salir de una
  subopción el encuadre cambia solo en un fotograma negro de la transición.
- [x] Cristal / icono que desaparece y popea al salir del menú de guardado (y de START): el
  objeto blanco de arriba a la derecha es el icono de la sala del minimapa; el juego lo
  destruía al abrir el menú y lo recreaba al cerrarlo. Ahora se mantiene. La conservación de los demás objetos de sala sigue pendiente de revisión en las transiciones.
- [ ] Subida de nivel: hueco neón arriba y abajo; algo popea justo antes. Necesita un estado antes de subir
  (el modo de depuración `Mode_Premire` no carga las franjas y los combates de depuración no la muestran).
- [ ] Menú de desbloquear puertas (síntesis de salas): subir un poco el menú
  principal para que no quede pegado abajo. Necesita un estado frente a una puerta cerrada.
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
- [ ] Transición de puerta: el usuario volvió a observar un flash cian; reproducir la ruta natural y corregirlo.
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
- [~] Texto dinámico de menús HD implementado; faltan números y revisión completa de prioridades.

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

- [~] Baterías: eventos US 194/194; combates US 209/209; combates como Riku 209/209;
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
- [~] Actualizar porcentajes de PROGRESS hasta el 100 % (91 % ahora).

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
- [ ] Otros crashes al alternar submenús: ampliar la ruta a Estado, Diario,
  naipes mapa y transiciones con distintas partidas.
- [ ] Animación superior de salida de submenús: revisar fotogramas de transición.
- [ ] Sprite a la izquierda antes/después de subir de nivel: reproducir y corregir.
- [ ] Números HD opcionales: determinar qué números son texto y cuáles forman parte
  de las ilustraciones de cartas; no sustituir dibujos completos por glifos.
- [x] Batería de eventos: `--wide` realmente selecciona view_mode=4. Antes
  generaba resultados etiquetados como panorámicos con view_mode=0.
- [~] Nombres de combos Sora/Riku: límite de seis entradas y gráficos del nombre
  actual corregidos bajo PLATFORM_PC. Compilan EU/US/JP; la prueba breve de un
  combate por personaje termina sin crash, falta comprobar una lista completa.
- [ ] Publicación siguiente: completar regresiones y las correcciones visuales
  pendientes antes de presentar v0.5 como versión verificada.

Validación reciente: subtítulos EU/JP en build/subtitles-current. La misma lectura
inválida de UpdateSpriteOam apareció tanto con modo forzado como entrando desde
START; la ruta de reproducción está en test_menu_cycles.py --edit.
