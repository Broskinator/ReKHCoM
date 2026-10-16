# Reparto vigente — v1.7.1 (16/10/2026)

Codex asume integración y publicación por pedido del usuario; Claude sin créditos.
Referencia actual: CODEX_COORDINACION.md. Los bloques anteriores son históricos.

- [x] Integrar rendimiento Codex sobre interfaz y escalado v1.7 de Claude.
- [x] Capturar ranura 1 de v1.6.7 en un directorio aislado.
- [x] Corregir interpolación PCM fuera de muestra: 24 regresiones pasan.
- [x] Optimizar síntesis PSG: 800.000 integrales idénticas a la referencia.
- [x] Preparar buffer de audio antes de reproducir y al cargar un estado.
- [x] Reducir espera activa; descartar omisión de copia tras detectar regresión ultrawide.
- [x] Recuperar numeración de píxeles v1.4, predeterminado x4 a 1080p y filtro real de enteras/intermedias.
- [x] Verificación regional, estados y capturas contra paquete público v1.7; seis casos Auto y 116 pruebas de zoom.
- [x] Publicada v1.7.1 unificada; ZIP/SHA-256 verificados y acceso local preparado.
- [x] Retiradas 14 releases del lote solicitado; todos los tags remotos intactos.

---

# Entrega v1.5 — reparto vigente (15/10/2026)

El usuario pide dejar continuidad para Claude y pushear cada bloque. Esta sección
reemplaza los repartos históricos de abajo. Rama local `claude/windows-5`, remoto
`origin/codex/v1.5-fixes`. No trabajar simultáneamente sobre render/menús.

**Leer primero [CONTINUIDAD_V15_HUD.md](CONTINUIDAD_V15_HUD.md).**
Codex entrega el bloque actual; Claude puede asumir los pendientes siguientes.
No se ha publicado v1.5 ni retirado v1.4. Guardados del usuario intactos.

- [x] Zoom: normal x4 a 1920×1080; pasos efectivos distintos, test_zoom.py en seis resoluciones. Push deaabb7.
- [x] Arena: continuar textura inferior antes de componer sprites; Ciudad de Paso, Agrabah y País de las Maravillas revisados. Push b0359b4.
- [~] Minimapa: anclaje de sprites a esquina implementado; queda matriz visual de todos los zooms/HUD. Incluido en deaabb7.
- [~] Síntesis: BG1 declarado UI para evitar panel perdido y bloques cian; captura de puerta real a 1920×1080 revisada, panel recuperado.
- [~] Selector: reaplicar desplazamiento de 8 líneas tras tres redibujados; guardia de división por cero en barra de una página. Falta revisión específica tras tutorial de puerta de evento.
- [~] Puerta: eliminado recorte rectangular del sombreado; entrada/salida ordinaria capturada. Validar todos los encuadres y suavidad del haz.
- [~] Submenús: escala entera independiente x5 a 1080p / x6 a 1440p. **Cabeceras/pies desacoplados pendientes**; se retiró el intento defectuoso que cortaba la imagen en franjas.
- [ ] Claude: corregir animación de salida de fondos/márgenes; no declarada terminada.
- [ ] Claude: matriz visual 16:9, 21:9 y 32:9, especialmente 5120×1440; todas las escalas válidas; EU/US/JP.
- [ ] Claude: resolver/atribuir tres fallos del test PPU de oclusión HUD, ver guía.
- [ ] Claude: changelog/README, ZIP sin ROM, acceso local v1.5, publicar y retirar v1.4 tras validar sustituta.
- [ ] Claude: tráiler nuevo con material de la versión terminada. No reutilizar capturas antiguas como evidencia.

Las compilaciones regionales y capturas se detallan en la guía. Los estados `[~]`
son parciales y no equivalen a aceptación visual del usuario.

---

# Reparto de pendientes â€” reordenado el 14 de octubre de 2026

Responsabilidades acordadas con el usuario para trabajar sin solaparse,
ordenadas por las barras de `status.json` que faltan para el 100 %.
`[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## CoordinaciÃ³n (14/10, 04:05)

- 14/10: Codex sin tokens. Mientras no vuelva a trabajar, Claude asume tambiÃ©n
  sus pendientes (widescreen field/battles/menus & events, nÃºmeros HD y su
  revisiÃ³n visual). Cuando Codex retome, se reparte de nuevo aquÃ­.

- Claude es el integrador principal: integra los commits de ambos en
  feat/native-movies, compila EU/US/JP antes de subir y prepara las releases.
  Solo se sube lo que compila en las tres regiones.
- NÃºmeros HD: Codex los cubre todos con el estilo 2 (tiles de BG en barajas y
  sprites de EXP); Claude no duplica. Pendiente: revisiÃ³n visual de colores.
- Herramientas de verificaciÃ³n: Codex tiene test_gameplay_*, test_ppu_conformance
  y test_wide_world_* sin subir; Claude ya subiÃ³ test_rooms, test_submenus,
  test_ppu, test_options y test_songs. Al integrar se unifican (sin borrar
  ninguna) y se anota aquÃ­ cuÃ¡l queda como referencia de cada barra.
- Ãšltimo: partida completa / TAS. DespuÃ©s del 100 %, la funciÃ³n experimental
  con las QoL de la versiÃ³n de PS2 (Re:Chain of Memories) en la de GBA.

## Reparto actual (14/10, tarde) â€” todas las barras al 100 %

CoordinaciÃ³n vigente por peticiÃ³n del usuario: Codex trabaja en el HUD HD y
deja commits locales con evidencia; solo Claude integra, pushea y actualiza
las releases. Prioridad del HUD: preliminar completo antes del ajuste fino.

| QuiÃ©n | Trabajo |
|---|---|
| Claude, junto con el usuario | Las 17 ventanas con mÃ¡rgenes laterales, una por una con su revisiÃ³n (lista abajo). Base: las versiones de la maÃ±ana del 14/10; solo mejoras puntuales. |
| Codex | Seguir mejorando el modo Re:CoM y sus QoL (port_ps2qol.c, la ventana de nueva partida en port_ui.c, kh_com_recom.ini). Regla: con el modo apagado o en la versiÃ³n Original nada cambia respecto al cartucho. |

### Las 17 ventanas (Claude + usuario, en orden)

Avance Codex (14/10): controles QoL y texto rÃ¡pido guardados por separado
por partida/banco en `kh_com_recom.ini`, compatible con registros anteriores.
La dificultad conserva daÃ±o cero y valores negativos. VerificaciÃ³n:
`python port/tools/test_recom.py` (80 combinaciones, cinco bancos, reinicio,
registros antiguos y aislamiento de Original). Sin cambios en los mÃ¡rgenes.

Nuevo experimental a cargo de Codex: HUD HD redibujado con IA y alfa real.
Formato preparado: atlas PNG RGBA + JSON, validaciÃ³n y vistas claro/oscuro
con `port/tools/pack_hd_hud.py`. Piloto integrado del panel HP enemigo con
acabado suavizado, PNG cacheado y opciÃ³n `hd_hud` en ambos menÃºs. Usuario
autoriza placeholders. Ver `HUD_HD_EXPERIMENTAL.md`; pruebas aisladas con
`test_hd_hud.py` superadas en EU/US/JP: recurso ausente y vista Original
idÃ©nticos a la referencia; barra/letras verdes conservan pÃ­xeles y posiciones.
El marco toma lÃ­mites nativos por frame, siguiendo la precisiÃ³n solicitada
por el usuario. No activar piezas con fondo residual o halos.

Base actual sincronizada: prerelease v0.8 y commits de mÃ¡rgenes/README del
14/10. Codex continÃºa encima: extracciÃ³n local de referencias HUD (25 piezas
en combate bÃ¡sico), dÃ­gitos de naipes con contornos CFF de KHMenu y oclusiÃ³n
por sprite. Barras propias en `status.json` â†’ `experiments`, sin modificar
el 100 % del port base. El panel enemigo preliminar ya incluye letras
HP/VT/LP/PV transparentes con relieve y relleno suavizado que conserva el
recorte nativo. PV tiene la esquina superior derecha de la P cerrada en
diagonal. Pruebas aisladas superadas en EU (es/de/it y Riku), US y JP;
pendientes: retratos, panel del jugador y restantes familias del inventario.

1. Diario â€” revisada (14/10)
2. Ver barajas / AÃ±adir-Quitar â€” revisada (14/10)
3. Estado â€” revisada (14/10)
4. Naipes mapa â€” revisada (14/10)
5. Tienda moguri â€” revisada (14/10)
6. Naipes mundo (quitar el tramado) â€” revisada (14/10)
7. Mapamundi â€” revisada (14/10)
8. Elige un mundo â€” revisada (14/10)
9. VÃ³rtice â€” revisada (14/10)
10. Bonus Ã©lite (selecciÃ³n de cartas) â€” revisada (14/10)
11. MenÃº START â€” revisada (14/10)
12. SÃ­ntesis de estancias (puerta y carta) â€” revisada (14/10)
13. Guardar â€” revisada (14/10)
14. Subida de nivel â€” revisada (14/10)
15. SelecciÃ³n de idioma â€” revisada (14/10)
16. Copyright y tÃ­tulo â€” revisada (14/10)
17. PelÃ­culas y entrada al combate â€” revisada (14/10)

## Barras pendientes y dueÃ±o (actualizado 14/10, 08:40 â€” total 98,9 %)

| Barra | % | Peso | DueÃ±o | QuÃ© falta para el 100 % |
|---|---|---|---|---|
| Gameplay verification | 100 | 10 | Claude | Cerrada por decisiÃ³n del usuario: las partidas completas quedan al feedback de los jugadores |

Cerradas hoy por Claude (con evidencia en port/progress/evidence/):
- Widescreen: battles 100 % â€” arenas US/JP 19/19, salida de combate con el
  fixture `battlewin` (combate de depuraciÃ³n y de campo), entrada con el
  corazÃ³n en su proporciÃ³n (battles_wide.txt).
- Widescreen: field maps 100 % â€” todas las salas de historia en EU/US/JP
  (118/120 salas Ãºnicas cada una; las otras dos son un encuentro antes de la captura,
  revisadas aparte), vestÃ­bulos y Bosque de los 100 Acres (fieldmaps_wide.txt).

Ya al 100 %: build, assets, memoria, BIOS, DMA, interrupciones, entrada, m4a,
PSG, PCM, cÃ³dec de bloques, pelÃ­culas, SRAM, reset, widescreen renderer,
widescreen field maps, widescreen battles, widescreen menus & events, estados
guardados, launcher, saltar
intro, versiones USA y japonesa, menÃº de opciones, PPU, texto HD.

## Acabado panorÃ¡mico (era de Codex; lo lleva Claude mientras Codex no vuelva)

- [x] Cristal del castillo: doce ciclos del menÃº conservan una sola tarea.
- [x] Cian en selecciÃ³n de carta de puerta; mÃ¡rgenes negros.
- [x] Entrada a combate: fondo difuminado, animaciÃ³n central en su proporciÃ³n.
- [x] Arenas como cinta de tiles: bandas de sus propios tiles elegidas por
  arena (btl_map.c), sin duplicar jefes ni motivos Ãºnicos; EU/US/JP revisadas.
- [x] Subida de nivel: entrada natural desde un combate de campo revisada
  fotograma a fotograma (sin pop, barras de borde a borde). El fondo que
  reaparecÃ­a a la izquierda al salir era defecto del port: el BG1 del panel
  daba la vuelta al mapa de 256 px; corregido con PortBgClip.
- [x] MenÃº de sÃ­ntesis de salas: panel 6 lÃ­neas por encima de la barra inferior.
- [x] Entradas/salidas de los seis submenÃºs (Ver barajas, Mapamundi, Naipes
  mapa, Naipes mundo, Estado, Diario) revisadas cada 3 fotogramas en EU; la
  salida de Ver barajas coincide con la GBA (el panel de naipes activos
  desaparece de golpe tambiÃ©n en el original). Mapamundi con footer completo.
- [x] Naipes mundo y Mapamundi en US/JP (fixture de sala + modo directo).
- [x] Cabeceras: en US/JP el margen mostraba letras ocultas del sprite (y, al
  pasar el tÃ­tulo a BG, columnas copiadas del cuadro). Ahora la banda usa el
  par de columnas mÃ¡s opaco y sin letras, guardado mientras la cabecera sigue en
  pantalla. Naipes mundo, Mapamundi, Estado, Diario y Barajas revisados en EU/US/JP.
- [x] Tabs del ojo en Estado y Barajas (EU/US/JP) y escenas con estirado
  (Bonus Ã©lite, Tienda moguri, Elige un mundo, VÃ³rtice) revisadas: barra al 100 %.
- [x] Sin estirado de pÃ­xeles: Diario, Elige un mundo, VÃ³rtice, Tienda moguri y
  Bonus Ã©lite usan mÃ¡rgenes hechos por el PPU con sus propias capas: el
  fondo (la capa BG mÃ¡s baja) sigue en espejo con sus tiles y paletas, como
  el flip horizontal del hardware; las capas de interfaz solo continÃºan como
  bandas donde llegan al borde. Sin tramado ni columnas estiradas.
- [x] Widescreen field: recorrido por mundo completo (arriba).

## Claude: verificaciÃ³n, regiones, texto HD, opciones y cierre del PPU

- [x] Gameplay verification: baterÃ­a de salas (test_rooms.py) 52/52 en EU: cada
  mundo en una planta temprana y una tardÃ­a y los vestÃ­bulos de las 13 plantas.
  Hoja de capturas en build/roomtest_eu/rooms.png para la revisiÃ³n de Codex.
- [x] Gameplay verification: estrÃ©s de submenÃºs (test_submenus.py) sobre una
  partida real: las seis entradas del menÃº de pausa, cuatro idas y vueltas
  cada una, 6/6 sin crash. Pendiente aparte: avisos de tutorial delante del
  texto de las cartas (tutorial de naipes mapa).
- [x] Gameplay verification: stutter de la subida de nivel medido con
  KHCOM_FRAMELOG: 16,7 ms estables, sin tirones ni fotogramas de lÃ³gica perdidos
  (renderizador por software; los picos vistos antes eran carga de otra prueba).
- [x] Gameplay verification: salas de Riku (Reverse/Rebirth) 88/88 en EU y de Sora
  en EU/US/JP 118/120 salas Ãºnicas (las otras dos, un encuentro antes de la captura).
- [ ] Gameplay verification: partida completa de Sora y de Reverse/Rebirth en
  las tres regiones (rutas reproducibles y problemas anotados).
- [x] USA/JP: combates JP 209/209 como Sora y como Riku; eventos 192/194 (los dos
  restantes se cortaron al parar, no fallaron). Barra al 100 %.
- [x] USA/JP: capturas regionales de arenas, salas, Naipes mundo y Mapamundi.
- [x] HD text: inventario (port/progress/HDTEXT_INVENTARIO.md).
- [x] HD text: nÃºmeros de barajas y EXP en HD con su color y tamaÃ±o (barra al 100 %).
- [x] Graphics options menu: test_options.py, 35 casos y el menÃº en EU (cinco
  idiomas), US y JP. Barra al 100 %.
- [x] PPU: regresiÃ³n de escenas (test_ppu.py, 12 escenas con referencia
  revisada). Barra al 100 %. Si un cambio de composiciÃ³n altera una escena,
  revisarla y regrabar con --update.
- [ ] PreservaciÃ³n: reproducir los casos de BUGS_GBA.md en EU/US/JP y separar
  tÃ©cnicas originales, anomalÃ­as del cÃ³digo y defectos del port.
- [x] Launcher e instalaciÃ³n: baterÃ­a 11/11.
- [x] Audio: comparaciÃ³n con emulador (mÃºsica, vÃ­deos), baterÃ­a de las 637
  canciones (port/progress/songs_report.txt), detalles del PSG, audio suave.
- [x] Estados guardados deterministas (3/3).
- [x] Releases v0.5, v0.6 y v0.7 (pre-releases pÃºblicas).
- [~] QoL: devolver cartas acumuladas y bancos de partidas hechos; aviso de
  puntos moguri, revancha de jefes y repetir tutoriales pendientes (no suman
  barras; despuÃ©s de las barras).
- [~] HD-2D experimental (versiÃ³n alternativa): profundidad de campo, bloom y
  gradaciÃ³n publicados; el pop-up de diorama estÃ¡ en un stash (bded017)
  y vuelve despuÃ©s de las barras.

## Reglas compartidas

- No modificar a la vez los mismos archivos; Codex trabaja principalmente en
  map/map_tasks/map_cell, card_level_up, perfiles de menÃº, PPU y presentaciÃ³n.
  Claude trabaja en harness y pruebas, opciones, launcher, audio y texto HD.
- Cada uno hace commit solo de sus propios cambios (por bloques si un archivo es
  compartido) y no toca el trabajo sin commitear del otro.
- No sobreescribir ejecutables o estados que el usuario estÃ© usando. La build
  compatible con sus estados estÃ¡ en build/priority-reference; las ranuras
  3/4/7/8/9 recientes en build/pcwork/states.
- Probar en directorios aislados y con pocos procesos mientras el usuario juega.
  No modificar partidas/configuraciÃ³n del usuario.
- No subir ROM ni recursos extraÃ­dos al repositorio.
- Subir porcentajes solo con evidencia y distinguir implementado de verificado.
- Usar emuladores Ãºnicamente para contrastar bugs o comportamientos dudosos del
  original. Validar acabados visuales y transiciones directamente en el port.
