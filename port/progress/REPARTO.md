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

## Barras pendientes y dueño

| Barra | % | Peso | Dueño | Qué falta para el 100 % |
|---|---|---|---|---|
| Gameplay verification | 85 | 10 | Claude + Codex | Partida completa de Sora y de Reverse/Rebirth |
| Widescreen: field maps | 87 | 8 | Codex | Salas con huecos o bordes de abismo; revisión por mundo |
| HD text | 94 | 3 | Claude | Números de menús de baraja (tiles de BG) con el compositor; oclusión general |
| Widescreen: battles | 95 | 6 | Codex | Cinta continua de arenas y bordes de abismo |
| Widescreen: menus & events | 93 | 4 | Codex | Salida de submenús, footer, subida de nivel (paneles y popeo) |

Ya al 100 %: build, assets, memoria, BIOS, DMA, interrupciones, entrada, m4a,
PSG, PCM, códec de bloques, películas, SRAM, reset, widescreen renderer,
estados guardados, launcher, saltar intro, versiones USA y japonesa,
menú de opciones, PPU.

## Codex: acabado panorámico (field, battles, menus & events)

- [x] Cristal del castillo: desaparece tras abrir/cerrar repetidamente su menú.
  Corregida la sustitución de tareas retenidas en salas fijas. Doce ciclos del
  menú conservan una sola tarea y el cristal visible; añadir uso real de A/B
  a la batería ampliada de navegación.
- [x] Cian en selección de carta de puerta (estado 7) y animación posterior
  de apertura (estado 9). El spotlight delimita su canvas y conserva márgenes
  negros; fixtures de spotlight/selección pasan sin cian en márgenes.
- [x] Entrada a combate: fondo difuminado como en las películas, animación
  central en su proporción. Revisado en EU/US/JP (test_levelup.py --entry).
- [~] Headers de Ver barajas, Añadir/Quitar, Naipes mapa, Estado/magias,
  Mapamundi, Naipes mundo y Diario desde los elementos originales, sin tomar
  columnas de la imagen final. Muestreo de una pieza limpia del asset original;
  Estado y barajas capturados; completar recorrido visual y cursor arriba/abajo.
- [~] Tabs del ojo a izquierda y derecha: relleno y contorno propios desde el
  tilemap/fotograma del tab. Completar captura de cada pestaña y transición.
- [~] Naipes mundo: fundido de pixel art tramado desde la paleta original.
  Revisado en EU (build/priority-regressions/20261013_231639_172982/world);
  capturas con partidas US/JP pendientes.
- [ ] Entradas/salidas de submenús sin ecos ni distorsión; Mapamundi con footer
  completo desde el inicio y al volver. Barras animadas de Mapamundi desde sus
  assets OBJ, revisadas en EU (build/priority-regressions/20261013_232146_939954/allmap).
  Otros submenús y ejecución regional pendientes.
- [~] Subida de nivel: paneles derechos revisados en reposo en EU a 640×360 y
  1280×720; relleno y contorno llegan al borde, sin cambio visual necesario.
  Evidencia: build/codex-resume-review/levelup/out_01600.bmp. Franjas unidas.
  Queda verificar el popeo en una entrada natural y la salida: en el fixture
  forzado, el fondo del panel reaparece a la izquierda (frames 2120–2136).
  No clasificarlo aún como defecto del port sin contrastar la ruta natural.
- [ ] Sustituir el estirado de píxeles por patrones o piezas de fondo donde sea
  viable; conservar los patrones aprobados de barajas y Mapamundi; revisar el
  píxel residual del margen derecho.
- [ ] Arenas como cinta circular de tiles: clasificar fondos repetibles y con
  elementos únicos; no duplicar jefes ni sprites ni cambiar colisiones.
- [ ] Widescreen field: recorrer por mundo las salas con huecos o abismo usando
  la batería de salas de Claude (abajo) y corregir lo que aparezca.
- [ ] Al cerrar cada barra: actualizar evidencia y porcentaje en status.json.

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
- [ ] Gameplay verification: partida completa de Sora y de Reverse/Rebirth en
  las tres regiones (rutas reproducibles y problemas anotados).
- [x] USA/JP: combates JP 209/209 como Sora y como Riku; eventos 192/194 (los dos
  restantes se cortaron al parar, no fallaron). Barra al 100 %.
- [ ] USA/JP: capturas regionales de los acabados panorámicos que Codex marque
  como revisados solo en EU.
- [x] HD text: inventario (port/progress/HDTEXT_INVENTARIO.md).
- [ ] HD text: números de los menús de baraja (escritos en tiles de BG) y
  regresión general de oclusión, coordinado con Codex en el compositor.
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
