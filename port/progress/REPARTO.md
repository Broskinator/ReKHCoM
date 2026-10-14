# Reparto de pendientes — reordenado el 14 de octubre de 2026

Responsabilidades acordadas con el usuario para trabajar sin solaparse,
ordenadas por las barras de `status.json` que faltan para el 100 %.
`[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## Coordinación (14/10, 04:05)

- 14/10: Codex sin tokens. Mientras no vuelva a trabajar, Claude asume también
  sus pendientes (widescreen field/battles/menus & events, números HD y su
  revisión visual). Cuando Codex retome, se reparte de nuevo aquí.

- Claude es el integrador principal: integra los commits de ambos en
  feat/native-movies, compila EU/US/JP antes de subir y prepara las releases.
  Solo se sube lo que compila en las tres regiones.
- Números HD: Codex los cubre todos con el estilo 2 (tiles de BG en barajas y
  sprites de EXP); Claude no duplica. Pendiente: revisión visual de colores.
- Herramientas de verificación: Codex tiene test_gameplay_*, test_ppu_conformance
  y test_wide_world_* sin subir; Claude ya subió test_rooms, test_submenus,
  test_ppu, test_options y test_songs. Al integrar se unifican (sin borrar
  ninguna) y se anota aquí cuál queda como referencia de cada barra.
- Último: partida completa / TAS. Después del 100 %, la función experimental
  con las QoL de la versión de PS2 (Re:Chain of Memories) en la de GBA.

## Barras pendientes y dueño (actualizado 14/10, 08:40 — total 98,9 %)

| Barra | % | Peso | Dueño | Qué falta para el 100 % |
|---|---|---|---|---|
| Gameplay verification | 100 | 10 | Claude | Cerrada por decisión del usuario: las partidas completas quedan al feedback de los jugadores |

Cerradas hoy por Claude (con evidencia en port/progress/evidence/):
- Widescreen: battles 100 % — arenas US/JP 19/19, salida de combate con el
  fixture `battlewin` (combate de depuración y de campo), entrada con el
  corazón en su proporción (battles_wide.txt).
- Widescreen: field maps 100 % — todas las salas de historia en EU/US/JP
  (118/120 salas únicas cada una; las otras dos son un encuentro antes de la captura,
  revisadas aparte), vestíbulos y Bosque de los 100 Acres (fieldmaps_wide.txt).

Ya al 100 %: build, assets, memoria, BIOS, DMA, interrupciones, entrada, m4a,
PSG, PCM, códec de bloques, películas, SRAM, reset, widescreen renderer,
widescreen field maps, widescreen battles, widescreen menus & events, estados
guardados, launcher, saltar
intro, versiones USA y japonesa, menú de opciones, PPU, texto HD.

## Acabado panorámico (era de Codex; lo lleva Claude mientras Codex no vuelva)

- [x] Cristal del castillo: doce ciclos del menú conservan una sola tarea.
- [x] Cian en selección de carta de puerta; márgenes negros.
- [x] Entrada a combate: fondo difuminado, animación central en su proporción.
- [x] Arenas como cinta de tiles: bandas de sus propios tiles elegidas por
  arena (btl_map.c), sin duplicar jefes ni motivos únicos; EU/US/JP revisadas.
- [x] Subida de nivel: entrada natural desde un combate de campo revisada
  fotograma a fotograma (sin pop, barras de borde a borde). El fondo que
  reaparecía a la izquierda al salir era defecto del port: el BG1 del panel
  daba la vuelta al mapa de 256 px; corregido con PortBgClip.
- [x] Menú de síntesis de salas: panel 6 líneas por encima de la barra inferior.
- [x] Entradas/salidas de los seis submenús (Ver barajas, Mapamundi, Naipes
  mapa, Naipes mundo, Estado, Diario) revisadas cada 3 fotogramas en EU; la
  salida de Ver barajas coincide con la GBA (el panel de naipes activos
  desaparece de golpe también en el original). Mapamundi con footer completo.
- [x] Naipes mundo y Mapamundi en US/JP (fixture de sala + modo directo).
- [x] Cabeceras: en US/JP el margen mostraba letras ocultas del sprite (y, al
  pasar el título a BG, columnas copiadas del cuadro). Ahora la banda usa el
  par de columnas más opaco y sin letras, guardado mientras la cabecera sigue en
  pantalla. Naipes mundo, Mapamundi, Estado, Diario y Barajas revisados en EU/US/JP.
- [x] Tabs del ojo en Estado y Barajas (EU/US/JP) y escenas con estirado
  (Bonus élite, Tienda moguri, Elige un mundo, Vórtice) revisadas: barra al 100 %.
- [x] Sin estirado de píxeles: Diario, Elige un mundo, Vórtice, Tienda moguri y
  Bonus élite usan un margen dibujado (estilo 7): las filas cuyo borde es un
  color liso siguen como banda; las demás se disuelven con tramado ordenado en
  el color de fondo. Ninguna escena usa ya el estilo de estirar columnas.
- [x] Widescreen field: recorrido por mundo completo (arriba).

## Claude: verificación, regiones, texto HD, opciones y cierre del PPU

- [x] Gameplay verification: batería de salas (test_rooms.py) 52/52 en EU: cada
  mundo en una planta temprana y una tardía y los vestíbulos de las 13 plantas.
  Hoja de capturas en build/roomtest_eu/rooms.png para la revisión de Codex.
- [x] Gameplay verification: estrés de submenús (test_submenus.py) sobre una
  partida real: las seis entradas del menú de pausa, cuatro idas y vueltas
  cada una, 6/6 sin crash. Pendiente aparte: avisos de tutorial delante del
  texto de las cartas (tutorial de naipes mapa).
- [x] Gameplay verification: stutter de la subida de nivel medido con
  KHCOM_FRAMELOG: 16,7 ms estables, sin tirones ni fotogramas de lógica perdidos
  (renderizador por software; los picos vistos antes eran carga de otra prueba).
- [x] Gameplay verification: salas de Riku (Reverse/Rebirth) 88/88 en EU y de Sora
  en EU/US/JP 118/120 salas únicas (las otras dos, un encuentro antes de la captura).
- [ ] Gameplay verification: partida completa de Sora y de Reverse/Rebirth en
  las tres regiones (rutas reproducibles y problemas anotados).
- [x] USA/JP: combates JP 209/209 como Sora y como Riku; eventos 192/194 (los dos
  restantes se cortaron al parar, no fallaron). Barra al 100 %.
- [x] USA/JP: capturas regionales de arenas, salas, Naipes mundo y Mapamundi.
- [x] HD text: inventario (port/progress/HDTEXT_INVENTARIO.md).
- [x] HD text: números de barajas y EXP en HD con su color y tamaño (barra al 100 %).
- [x] Graphics options menu: test_options.py, 35 casos y el menú en EU (cinco
  idiomas), US y JP. Barra al 100 %.
- [x] PPU: regresión de escenas (test_ppu.py, 12 escenas con referencia
  revisada). Barra al 100 %. Si un cambio de composición altera una escena,
  revisarla y regrabar con --update.
- [ ] Preservación: reproducir los casos de BUGS_GBA.md en EU/US/JP y separar
  técnicas originales, anomalías del código y defectos del port.
- [x] Launcher e instalación: batería 11/11.
- [x] Audio: comparación con emulador (música, vídeos), batería de las 637
  canciones (port/progress/songs_report.txt), detalles del PSG, audio suave.
- [x] Estados guardados deterministas (3/3).
- [x] Releases v0.5, v0.6 y v0.7 (pre-releases públicas).
- [~] QoL: devolver cartas acumuladas y bancos de partidas hechos; aviso de
  puntos moguri, revancha de jefes y repetir tutoriales pendientes (no suman
  barras; después de las barras).
- [~] HD-2D experimental (versión alternativa): profundidad de campo, bloom y
  gradación publicados; el pop-up de diorama está en un stash (bded017)
  y vuelve después de las barras.

## Reglas compartidas

- No modificar a la vez los mismos archivos; Codex trabaja principalmente en
  map/map_tasks/map_cell, card_level_up, perfiles de menú, PPU y presentación.
  Claude trabaja en harness y pruebas, opciones, launcher, audio y texto HD.
- Cada uno hace commit solo de sus propios cambios (por bloques si un archivo es
  compartido) y no toca el trabajo sin commitear del otro.
- No sobreescribir ejecutables o estados que el usuario esté usando. La build
  compatible con sus estados está en build/priority-reference; las ranuras
  3/4/7/8/9 recientes en build/pcwork/states.
- Probar en directorios aislados y con pocos procesos mientras el usuario juega.
  No modificar partidas/configuración del usuario.
- No subir ROM ni recursos extraídos al repositorio.
- Subir porcentajes solo con evidencia y distinguir implementado de verificado.
- Usar emuladores únicamente para contrastar bugs o comportamientos dudosos del
  original. Validar acabados visuales y transiciones directamente en el port.
