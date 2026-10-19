# Cola vigente — pasada al limpio, 19/10/2026

Única cola de trabajo (los demás .md son evidencia). Numeración histórica
conservada para poder rastrear. Reglas del usuario:

- Claude y Codex commitean/pushean a main; si uno lleva más de 10 min sin commits ni
  archivos tocados, el otro toma sus puntos anotando hora y evidencia aquí.
- Todo lo visual se entrega como copia jugable + .bat y espera aprobación del
  usuario; lo aprobado se documenta en LOGICA_APROBADOS.md; método: PRUEBAS_VISUALES.md.
- Lo publicado sale de la cola. Validación en hardware real NO es pendiente:
  llega como reporte (sección final).
- Nunca borrar carpetas ReKHCoM-vX sin pedirlo el usuario (18/10: autorizó borrar
  las antiguas dejando 3; respaldo de guardados en backups/).

## Reparto actual (19/10, mañana)

| Quién | Puntos |
|---|---|
| **Claude** | 1) v1.9.9 estable con release_check.py; 2) 51–54 alphas de consolas |
| **ChatGPT/Codex** | 1) 71 logros oficiales restantes (62/90; jefes: `ra_boss_dryrun.py` filas 1 y 6–13): validar con BizHawk las cadenas gBtlWork/gCardBattleState→heap, Pooh 0x0203C9D4, piso 0x0203CB86, IWRAM 0x03002D00 y activarlos en sOfficialVerified (port_achievements.c); herramientas ra_layout.py, ra_layout_check.py, ra_witness.py, bizhawk_*.lua; 2) 61–62 escenas de Ciudad de Paso; 3) 39 rutas completas; 4) 16–24 arte de jefes |

Hecho del 19/10 (madrugada/mañana): v1.9.8 publicada y pasada a prerelease (no aprobada
como estable); 23/90 logros oficiales activos; menú de Opciones ya no lee reproductores
nulos (crash en Android); confirmar/cancelar en la pausa de escenas con los sonidos del
juego; truco «llenar colección» quitado (rompía el menú); PS5 vuelve a compilar
(estados silenciosos para las revanchas); la web publica logo y portada; release_check.py
bloquea publicaciones incompletas (web, PS5, versiones).

## Pendientes para el futuro

- **71 RetroAchievements:** 28/90 oficiales activos (19/10: +5 jefes validados con
  `ra_boss_dryrun.py`, que evalúa las 90 definiciones oficiales sin conexión con
  KHCOM_RA_DRYRUN y derrota al jefe por la vía de daño del propio juego (`bosskill`):
  Guard Armor 160363, Jafar 160365, Hades 160369, Úrsula 160376, Parasite Cage 160367
  saltan justo al derrotarlo y nunca en el control. Trickmaster 160364 y Oogie no saltan
  (condición extra; Trickmaster resetea con 0x02034000 = sKeyReleaseSelect); filas 7–13
  sin probar.) Antes: 23/90 (los que solo leen GameState,
  teclas, modo y el estado del título; añadido 160372 de Pooh por Codex). Faltan 67: dependen de cadenas desde
  gBtlWork/gCardBattleState hacia el heap (48+7), heap fijo 0x02033BC8 (3), Pooh
  0x0203C9D4, piso 0x0203CB86, IWRAM 0x03002D00 y 0x0203A6xx. Validar comparando la
  RAM del port con el original (BizHawk) en las mismas situaciones y activarlos en
  sOfficialVerified (port_achievements.c). Herramientas: ra_layout.py, ra_layout_check.py,
  ra_witness.py, audit_ra_references.py.
- **51–54 consolas:** alphas de Vita, PS4 y Switch (PS5 ya existe por VM).
- **61–62:** escenas de Ciudad de Paso con márgenes incompletos (requieren aprobación).
- **16–24:** arte de jefes y escenas, de a uno con antes/después.
- **39:** rutas completas Sora y Reverse/Rebirth.

## Cerrados hoy (referencia)

72 paridad de cristal en 4 semillas (19/10, `crystal_parity.py`): 0x2242C2F1, 0x3A4A4A40,
0x5F5E1000 y 0xA344A3F9 dan 3 posiciones distintas y el original (BizHawk, US) tiene el
cristal en la misma posición que el port en las 4.

72 cristal/faroles (prueba definitiva en el GBA, 19/10): con la partida del usuario y solo la
semilla de la sala cambiada a la de su prueba del port (0x2242C2F1, save_reseed.py), el
cartucho original (BizHawk) pone el cristal en (57344, 61440), exactamente donde el port; la
posición anterior desaparece. El código máquina del cartucho (CreateMapRoom en 0x080DF66E)
carga 0x03007480 dos veces, MUL y lo guarda como semilla: depende del momento de usar la
carta, también en el GBA. Las pruebas previas con el .vbm estaban contaminadas por la SRAM del
usuario (BizHawk identifica el juego por hash). Salas fijas (halls, historia) usan tablas de la
ROM. El port no genera salas imposibles en GBA. Partida BizHawk del usuario respaldada y
restaurada (sha1 verificado). Cristales y faroles en otra posición que el emulador (reporte del 19/10): la lógica es la
del cartucho. En la RAM del original (BizHawk) gFrameCounter (0x03007480, el que lee
CreateMapRoom según la ROM) valía 56129 y la semilla de la sala 0x90FAECF1 = 49319²: la
sala se creó en el fotograma 49319 de esa partida. En la prueba del port el usuario entró
en el 23975 (semilla 0x2242C2F1). Con la misma semilla el port pone el cristal y el farol
donde el original (comprobado en 63). Cambia con el momento de usar la carta, también en
el GBA. Limpieza (44, 19/10): fuera los worktrees khcom-linux, khcom-codex-perf (su rama queda), khcom-fix y khcom-n5 (sus cambios sueltos en backups/*.patch) y las carpetas de prueba de build; se conservan khcom-ps5, khcom-claude, khcom-wide-review, khcom-codex y las ReKHCoM-vX. Copia «Probar ReKHCoM v1.9.8 en curso.bat» con la build de main y partida (archivo 1 nivel 3 del usuario, archivo 2 editado a nivel 50). Crash de Riku en Android (68): la apertura de Riku leía obj.anim nulo (bug original); en PC lo cubría el manejador de fallos, en Android no. BIOS_PTR en esa lectura y otros 11 accesos nulos documentados (d79640b01). Confirmado en el emulador x86: nueva partida de Riku con texto HD pasa el fotograma 2400 sin crash. #21 de Codex en main (22b115878). Pausa en escenas (65): toque corto de START → Volver al juego / Saltar escena (con confirmación, «No» preseleccionado); el salto usa el mismo camino que mantener START (respeta las 3 escenas no saltables; recompensas y banderas intactas, probado en EVENT 0:5); mantener START sigue saltando como el original. Franjas lisas en los bordes de escenas (barrido find_blank_margins, 78 tomas): revisadas, son el fin real de la sala (vacío/color de fondo del juego, lógica aprobada en 56); sin cambios. Clic del modo debug en Opciones con el sonido del juego restaurado exacto (58, 55e1e30f0) y banderas de idiomas en el launcher (67, 4a5c89ef5): el usuario delegó la aprobación en Claude (19/10), verificados por medición y captura; salen en v1.9.6. Lógica Mapa GBA en Trucos → Modo debug, con panel explicativo (69): APROBADO 19/10, en main (8a74f004b), sale en v1.9.6. Márgenes sin pintar en las escenas del tutorial (56/56b) en f5ddc75b9, sale en v1.9.6. Menú de Opciones a toda la altura (57) y aviso moguri a Experimental (64), publicados en v1.9.5. 29 no se reproduce (Mapamundi correcto en los 12 mundos a 16:9; la revisión de objetos de sala la cubren los 36/36 menús de 28): cerrado. 36–37 web probada por el usuario en navegador real: va bien. Mando (49) y 057 playa (12) aprobados por el usuario. Guardados y opciones (5–9), #17 borde de Riku (10), #18 Mapamundi (11), 061 hielo
y 074 Monstro (13–14), remapeo y gatillos (25), submenús (28), fuente CJK Linux
(34), PS5 (38), táctil (45), transición de Opciones (50), regresión sincorazones (4).
Publicado en Nightly 5 (18/10, v1.9.3-nightly.20261018.5): #17 borde de Riku, logros locales,
arte del mando (49 aprobado), transición de Opciones, CJK Linux, corrección Linux #19.
Publicados antes: Nightly 4 (trucos, revanchas, aviso moguri, editor de partidas,
#16, diagrama de mando v1). Ver NIGHTLY4_PUBLICACION.md y NIGHTLY5_PUBLICACION.md.

## Futuro muy lejano (no cuenta como pendiente)

- 057 playa (12, aprobada): igualar los tiles del lado superior derecho con el
  izquierdo y que el pasto no tenga motivos cortados. Solo si algún día sobran tokens.

## Fuentes

LOGICA_APROBADOS.md, PRUEBAS_VISUALES.md, CONTROLADOR_LOGICA.md,
MENU_TRANSICIONES_LOGICA.md, ESCENAS_REVISION_AMPLIA.md, CODEX_TAREAS.md,
CODEX_RESPUESTAS.md, LINUX.md, WEB_PENDIENTES.md. Pruebas e imágenes en build/;
no versionar ROM, saves ni savestates.

19/10, instrucción del usuario: dejar los fondos de lado; Codex retoma 27 y
commitea avances comprobables aproximadamente cada tres minutos. Las capturas
actuales de 61/62 están en build/revision-61-62-actual; no hay nueva solución
ni aprobación visual. No confundir estos pendientes con las escenas 061/062.

71, Codex 19/10: corregido el anclaje sBgFx (era gBtlWork por error) y activado
160372 tras validar su grafo completo en 300 frames controlados. 22/90 = 24,4 %.
Evidencia: RETROACHIEVEMENTS_71_VALIDACION.md. No es un premio real enviado.

71, Codex: 15582 activado tras probar la transición de sala (gMapFloorState.room,
byte +6, no piso). 23/90 = 25,6 %, quedan 67. 800 frames de condiciones reales
comprobados entre 15582 y Pooh; controles negativos sin activación.
