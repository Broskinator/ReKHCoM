# Pendientes

Lista de trabajo pedida por el usuario, con sus ejemplos. Se actualiza en cada
commit: `[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## Pantallas y menús 16:9

- [x] Menú START: popeaba al cerrarlo y se "achicaba" al entrar en una subopción.
  Verificado fotograma a fotograma: abrir/cerrar sin saltos; al entrar/salir de una
  subopción el encuadre cambia solo en un fotograma negro de la transición.
- [x] Cristal / icono que desaparece y popea al salir del menú de guardado (y de START): el
  objeto blanco de arriba a la derecha es el icono de la sala del minimapa; el juego lo
  destruía al abrir el menú y lo recreaba al cerrarlo. Ahora se mantiene. Los objetos de sala
  (cristal, barriles, saltos) también se conservan durante los menús en vez de recrearse.
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
  [ ] devolver a la mano las cartas acumuladas (Sora); [ ] aviso de puntos moguri en el campo;
  [ ] bancos de guardado (varios .sav); [ ] revancha de jefes; [ ] repetir tutoriales.
- [~] Fondos blancos fuera del mapa del castillo → negro. En el vestíbulo ya sale negro;
  si aparece blanco en otra sala, hace falta un estado allí.
- [x] Transición de puerta y síntesis de salas: cian → negro (barras de estirar borde).
- [x] Barras laterales: título/menú inicial blanco, copyright crema, idioma con
  grano (sin copiar emblemas), pantalla de carga estirando el último píxel.
- [x] Arenas de combate: el escenario se repite más allá de los bordes (209 combates revisados).

## Combate

- [x] Subir el HUD de cartas (la carta sigue por debajo del borde): 8 px por
  defecto, opción "Subir cartas en combate" (0–16 px) en el juego y el launcher.
- [x] Batería de los 209 combates del menú de depuración (`port/tools/test_battles.py`): 209/209 sin cuelgues.

## Texto

- [x] Texto HD con la fuente KHMenu (cabe en los recuadros, acentos y ¿¡ nativos).
- [ ] Comic Hearts para las cinemáticas (subtítulos de las películas).
- [x] Recuadro de mensaje sin retrato descentrado (ranura 2 del usuario).
- [x] Texto HD en la versión JP (Yu Gothic UI sobre la rejilla del juego).
- [ ] Texto HD en las fuentes de menú (listas de Estado, etc.).

## Sonido

- [x] Música del estado 1 (Coliseo, `BGM_EVENT1`): intro → silencio → intro.
  Es así en la ROM: la última nota acaba en el tick 748 de 768 y la reverberación
  (40/127) se apaga en pocos fotogramas; el port reproduce los datos tal cual.

## Pruebas y publicación

- [~] Baterías: eventos US 194/194; combates US 209/209; combates como Riku 209/209;
  eventos y combates JP en curso.
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
