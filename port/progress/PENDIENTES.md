# Pendientes

Lista de trabajo pedida por el usuario, con sus ejemplos. Se actualiza en cada
commit: `[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## Pantallas y menús 16:9

- [~] Menú START: popeaba al cerrarlo y se "achicaba" al entrar en una subopción
  (la capa de pantalla ahora se mantiene un fotograma más; falta verificación visual).
- [ ] Cristal de guardado: popea al salir y el cristal desaparece.
- [ ] Subida de nivel: hueco neón arriba y abajo; algo popea justo antes.
- [ ] Menú de desbloquear puertas (síntesis de salas): subir un poco el menú
  principal para que no quede pegado abajo.
- [x] Menú del Vórtice y "Elige un mundo": altura completa con bordes estirados (las
  capturas con barras negras eran de la build anterior; "Elige un mundo" no llenaba la altura).
- [~] Submenús en 16:9 ("todo todo"): Diario, Ver barajas, Estado, Mapamundi, Naipes:
  todos a altura completa con bordes estirados, como la pantalla de carga.
- [ ] Revisar en cada pendiente si el port de Vita u otro recomp tiene una solución aplicable.
- [ ] Fondos blancos fuera del mapa del castillo → negro o color que corresponda.
- [x] Transición de puerta y síntesis de salas: cian → negro (barras de estirar borde).
- [x] Barras laterales: título/menú inicial blanco, copyright crema, idioma con
  grano (sin copiar emblemas), pantalla de carga estirando el último píxel.
- [~] Arenas de combate: el escenario se repite más allá de los bordes.

## Combate

- [x] Subir el HUD de cartas (la carta sigue por debajo del borde): 8 px por
  defecto, opción "Subir cartas en combate" (0–16 px) en el juego y el launcher.
- [~] Batería de los 209 combates del menú de depuración (`port/tools/test_battles.py`).

## Texto

- [x] Texto HD con la fuente KHMenu (cabe en los recuadros, acentos y ¿¡ nativos).
- [ ] Comic Hearts para las cinemáticas (subtítulos de las películas).
- [x] Recuadro de mensaje sin retrato descentrado (ranura 2 del usuario).
- [ ] Texto HD en la versión JP y en los menús.

## Sonido

- [~] Música del estado 1 (Coliseo, `BGM_EVENT1`): intro → silencio → intro.
  La ROM tiene ~1,1 s sin notas antes del punto de bucle y el port la reproduce
  igual; falta compararla con mGBA para confirmar que el original suena así.

## Pruebas y publicación

- [ ] Baterías de eventos US/JP (rehacer con la build actual).
- [ ] Launcher: remapeo de controles.
- [ ] Release v0.3 (pre-release, no Latest) con launcher.
- [ ] Instalar la build nueva en build/pc cuando el juego esté cerrado
  (sin tocar ini, save ni estados del usuario).
- [ ] Actualizar porcentajes de PROGRESS hasta el 100 %.
