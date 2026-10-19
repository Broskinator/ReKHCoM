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

## Reparto actual (19/10 — pasado al limpio)

| Quién | Puntos (orden) |
|---|---|
| **Claude (Opus)** | 1) 44 limpieza; 2) copia + .bat de la última build con partida avanzada; 3) 27 logros (Codex apagado: Claude lo termina); 4) 55+70 web y launcher como un solo punto; 5) v1.9.8 estable en TODAS las plataformas presupuestadas, con las alphas de consolas 51–54 al mismo tiempo; 6) 39 rutas |
| **ChatGPT/Codex** | apagado (19/10): Claude toma 27, 55 y 70. Queda para Codex: 2) 27 logros (memoria/set oficial, Level 1); 3) 55 web (espera revisión visual); 4) 70 rediseño web y launcher (asignado a Codex por el usuario, 19/10); 5) 61–62 escenas de Ciudad de Paso; 6) 16–24 arte de jefes |

Si uno lleva más de 10 min sin actividad, el otro toma sus puntos anotando hora y evidencia.

## 1. Por publicar

- **v1.9.8 estable (19/10, EN ESPERA hasta que el usuario diga; quedan mejoras):** todo lo de main + #21 de Codex; Windows, Linux,
  Steam Deck, Anbernic, Android y Web. Las demás releases quedan solo como tags.
  Al publicar: responder #20 (Android) y #21.

## 2. Codex

- **#21 (issue, Codex, listo para integrar):** corregido crash de Marluxia 2.
  Truco/editor excluyen la carta interna; las vistas de Sora/Riku/intercambio
  comprueban rango y descripción nula para guardados existentes. EU/US/JP
  compilan; 8.550 vistas dirigidas pasan. Evidencia: ISSUE_21_MARLUXIA.md.
  Claude: incluir en v1.9.8 y responder/cerrar el issue al publicar.
- **27 logros:** catálogo, imágenes, 16 disparos por región y menú in-game listos; falta
  validar memoria/set oficial y Level 1.
- **55 web:** opciones del launcher en pestañas y estética del juego; commiteada, espera
  revisión visual (WEB_55_REVISION.md).
- **70 (Codex, tras 27):** mejorar web y launcher con criterio estético, guías de diseño web,
  referencias de diseño, recrear desde captura y verificación con navegador
  (plugins sugeridos: audit-suite, playwright, designs-drift, Design).
- **61 (pendiente, sin aprobación):** plaza de León; captura actual 19/10 a x3/1080p confirma cortes de extremos.
- **62 (pendiente, sin aprobación):** casa de Ciudad de Paso; captura actual 19/10 a x3/1080p confirma corte derecho y puntas sin completar.
- **16–24 (al final, de a uno con antes/después):** arte de jefes y escenas: Darkside
  (WIP en wip/darkside-backdrop-20261018), Oogie, Jafar, Maléfica dragón, Úrsula,
  Marluxia final, Alicia/Trickmaster, resto de candidatas, tira de arenas GBA.

## 3. Claude

- **68:** Android, Riku: crash al iniciar partida nueva (con texto HD). Replicar con el
  APK x86 de prueba en el emulador (AVD en D:/ReKHCoM-build/android-avd), sin la sesión
  de juego del usuario abierta.
- **44:** limpieza de worktrees y carpetas build viejas (preguntar antes de borrar).
- **39 (al final):** rutas completas Sora y Reverse/Rebirth con el .vbm como guía por
  tramos y el bot (KHCOM_BOT).
- **43:** mantener esta cola tras cada cierre.

## 4. Claude: consolas (antes del otro chat, cerrado 19/10)

- **51 PS Vita:** de ggdrn/KH_COM_VITA solo la forma de compilar (VitaSDK + vitaGL); el
  resto es nuestro. Fases: referencia en Vita3K → punteros reubicables → .vpk alpha.
- **52 PS4** (reusar el guest de port/ps5 con OpenOrbis/GoldHEN; probar en shadPS4),
  **53 Switch** (64 bits o proceso de 32), **54 PS3** (big-endian, muy improbable).

## Esperan reportes (no cuentan como pendientes)

- **63:** cristal de guardado y colisión: arreglado con la lógica del juego; se confirma
  jugando.
Mando físico, Linux/Steam Deck, Anbernic RG34XX, Android/Odin 2 y móvil real:
se reabren solo si un reporte muestra un fallo.

## Cerrados hoy (referencia)

Crash de Riku en Android (68): la apertura de Riku leía obj.anim nulo (bug original); en PC lo cubría el manejador de fallos, en Android no. BIOS_PTR en esa lectura y otros 11 accesos nulos documentados (d79640b01). Confirmado en el emulador x86: nueva partida de Riku con texto HD pasa el fotograma 2400 sin crash. #21 de Codex en main (22b115878). Pausa en escenas (65): toque corto de START → Volver al juego / Saltar escena (con confirmación, «No» preseleccionado); el salto usa el mismo camino que mantener START (respeta las 3 escenas no saltables; recompensas y banderas intactas, probado en EVENT 0:5); mantener START sigue saltando como el original. Franjas lisas en los bordes de escenas (barrido find_blank_margins, 78 tomas): revisadas, son el fin real de la sala (vacío/color de fondo del juego, lógica aprobada en 56); sin cambios. Clic del modo debug en Opciones con el sonido del juego restaurado exacto (58, 55e1e30f0) y banderas de idiomas en el launcher (67, 4a5c89ef5): el usuario delegó la aprobación en Claude (19/10), verificados por medición y captura; salen en v1.9.6. Lógica Mapa GBA en Trucos → Modo debug, con panel explicativo (69): APROBADO 19/10, en main (8a74f004b), sale en v1.9.6. Márgenes sin pintar en las escenas del tutorial (56/56b) en f5ddc75b9, sale en v1.9.6. Menú de Opciones a toda la altura (57) y aviso moguri a Experimental (64), publicados en v1.9.5. 29 no se reproduce (Mapamundi correcto en los 12 mundos a 16:9; la revisión de objetos de sala la cubren los 36/36 menús de 28): cerrado. 36–37 web probada por el usuario en navegador real: va bien. Mando (49) y 057 playa (12) aprobados por el usuario. Guardados y opciones (5–9), #17 borde de Riku (10), #18 Mapamundi (11), 061 hielo
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
