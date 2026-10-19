# Cola vigente — pasada al limpio, 18/10/2026

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

## Reparto actual (18/10, noche)

| Quién | Puntos | Orden sugerido |
|---|---|---|
| **Claude (Opus)** | 57 menú de Opciones a toda la altura; 58 sonidos del modo debug; 63 colisiones; 64 moguri a Experimental; 65 pausa y saltar escena; v1.9.4 y #19; 44 limpieza; 43 cola; 51–54 consolas (otro chat); 39 rutas (al final) | 1) 58 (corto), 2) 63, 3) 65, 4) 44 con permiso, 5) 39 |
| **ChatGPT/Codex** | 59 caras flotando en escenas; 60 barras del título; 40 audio PSG; 55 página web con las opciones del launcher y estética del juego; después 61–62 y 16–24 | Orden explícito del usuario: **55 → 27** (59 y 60 aprobados; 40 sin fallo actual). Los demás quedan después. |

Reglas de continuidad y orden por tamaño: lo corto primero; arte de jefes y rutas
completas al final.

## 1. Por publicar

- **v1.9.6 (pronto):** con los ports de vuelta (Linux, Steam Deck, Anbernic, Android,
  Web; PS5 por su VM) y lo que se integre hasta entonces (59 retratos de Codex ya en
  main). v1.9.5 salió el 18/10 como prerelease, solo Windows; la estable sigue v1.9.2.
  Prueba Android para el usuario: v1.9.6-android-test.1 (reserva por regiones #20 y
  retratos #59). El usuario confirma que en su Android corre bien. **Responder el
  issue #20 recién al publicar la próxima estable.**

## 2. Codex

- **16–24:** arte de jefes/escenas, al final y de a uno con antes/después:
  16 Darkside (WIP en rama wip/darkside-backdrop-20261018, fuera de main),
  17 Oogie, 18 Jafar, 19 Maléfica dragón, 20 Úrsula, 21 Marluxia final,
  22 Alicia/Trickmaster (tablero inferior), 23 resto de candidatas,
  24 tira de fondo de arenas GBA.
- **26:** CERRADO 18/10: issue #2 (funciones del remake) cerrado por el usuario como
  mod muy específico, fuera de alcance por ahora.
- **27:** RetroAchievements: hay logros locales y catálogo oficial experimental;
  falta login real y validar memoria/set; Level 1 pendiente.
- **55 (nuevo, 18/10):** página web: llevar las opciones del launcher a sus
  pestañas (gráficos, sonido, mejoras, controles, editor) y rehacer la estética
  general para que calce con el arte del juego (estilo del menú debug ya usado en
  el launcher y en Opciones). Entrega visual: capturas antes/después y aprobación.
- **40:** sin fallo actual reportado: el usuario confirma que el audio funciona bien
  (18/10). Investigación histórica v1.6.7 apartada; no se cambió la síntesis.
  Reabrir solo ante una reproducción actual concreta.
- **59 (bug grande, 18/10, PRIORIDAD):** caras flotando en las escenas: el retrato
  **APROBADO por el usuario el 18/10. Implementado y commiteado (`acd85c818`).** Reproducido DiZ a color y Riku sin
  ventana; posición de espera oculta y recorte del retrato al área GBA de su ventana.
  EU/US/JP compilan; 12 comparaciones / 24 ejecuciones pasan, 48 pares GBA idénticos.
  Copia separada + BAT «Probar retratos 59 - Riku/Sora» en la raíz. Detalle:
  RETRATOS_59_LOGICA.md. Encargo original:
  del diálogo aparece suelto, sin su ventana de texto, en casi todas las escenas
  del juego (el usuario creía que era normal). En una escena de Riku (círculo gris
  sobre fondo blanco, Riku agachado en el centro) el retrato de Riku sale flotando
  a la derecha y hasta «revelado». Comparar con el juego original, encontrar la
  causa (orden de ventana/retrato, capa HD, márgenes anchos u oclusión) y
  corregirlo en todas las rutas (Sora y Reverse/Rebirth, EU/US/JP).
  Detalle del usuario: pasa ANTES de que hablen. Ejemplo: Riku en el círculo
  gris; DiZ aparece a la izquierda con su cara a color (revelado) y, cuando
  entra la ventana «Duerme. Aquí, entre la luz y la oscuridad», pasa a la
  silueta negra correcta. Hipótesis: en la GBA el retrato espera fuera de
  pantalla (x negativa) mientras entra la ventana, y la vista amplia lo muestra
  en el margen; ocultar retratos fuera de 240x160 mientras su ventana no está.
- **60 (APROBADO por el usuario el 18/10):** copia + BAT «Probar titulo 60»
  en la raíz. Lógica y evidencia en TITULO_60_LOGICA.md. Pantalla de título: la barra
  «PULSA START» con su transparencia debe llegar hasta el borde derecho, y el
  borde del pie «L ◀ BRILLO ▶ R» hasta el izquierdo, en todas las resoluciones.
- **61 (18/10):** revisar escenas en zoom x3 (integra x2) a 1080p, que muestra
  más que x4: Ciudad de Paso, plaza con León (dos escenas casi iguales: «Sí, más
  o menos. Ya iré aprendiendo…» y la anterior): bien a la izquierda, a la derecha
  falta completar fuera de lo que cubría x4.
- **62 (18/10):** escena en la casa de Ciudad de Paso (Yuffie, Aerith, León;
  «¡Sí, exacto! Como si no nos conociéramos…», «¿Cómo es eso?»): faltan las dos
  puntas de la habitación y hacia arriba. Comprobar si está en la lista de 44
  candidatas; si no, agregarla.
- **2:** CERRADO por decisión explícita del usuario: trámite de coordinación descartado. No requiere implementar ni volver a contactar el chat de sincorazones.

## 3. Claude

- **58 (18/10):** añadir los sonidos de las luces del modo debug al menú de
  Opciones, un poco más bajos de volumen. Intento en stash
  `claude-58-menu-click-music-regression`: el clic suena, pero pausar la música con
  m4aMPlayStop corta las notas largas (al volver suena más baja) y deja un tono
  sostenido del PSG con el menú abierto. Falta pausar sin soltar notas.
- **63 (18/10, colisión) — hecho, falta tu prueba:** se quitó la lógica propia del
  port que conservaba y recreaba cristales/gimmicks en los menús (4e6bb4aad,
  d3c5f41e2) y el cristal de prueba del harness; ahora siguen el flujo del juego.
  Con la semilla de tu partida original (0x90faecf1, RAM de BizHawk) el port pone el
  cristal en el mismo lugar que el original (73728,65536, junto al farol): la
  posición depende de la semilla de la sala (fotograma al usar la carta), igual que
  en el GBA. Detrás del cristal, izquierda avanza; 8 ciclos de menú de guardado y 6
  de pausa: el cristal sigue (sala aleatoria y castillo). Confirmar con una partida.
  Revisión de todo el mapa contra el decomp original: el icono del mapa de la sala
  también vuelve a su ciclo original. Los Heartless del campo siguen desapareciendo
  según la vista ancha (v1.9.1): el usuario decidió mantenerlo (18/10).
- **69 (19/10, visual, espera aprobación):** Trucos → Modo debug → "[ReKHCoM] Lógica
  Mapa GBA" (apagado por defecto): los Heartless del campo vuelven a los límites de
  240x160 del cartucho. Al ponerse sobre la opción, un panel sobre el arte de Sora la
  explica con dos diagramas (ReKHCoM / GBA), en los 6 idiomas.
- **68 (bug, 18/10):** Android (APK de prueba v1.9.6): iniciar partida con texto HD
  activado crashea. Replicar en el emulador (AVD en D:/ReKHCoM-build/android-avd).
- **66 (18/10):** Android: la app aún pregunta el idioma del juego al empezar, pero
  se elige dentro del juego. Quitar esa pregunta; dejar más escondida una opción
  solo para cambiar el idioma de la interfaz/launcher.
- **67 (18/10):** launcher de PC: banderas de los idiomas en la página principal
  de elegir versión (EU/US/JP).
- **65 (18/10, nuevo):** menú de pausa en las escenas habladas («cinemáticas»):
  centrado, con el estilo de botón del menú (la luz), dos opciones: Volver al
  juego y Saltar escena; Saltar pide confirmación. Implementar el salto de escena
  sin romper nada (estado, banderas de historia, música, ruta Sora y Riku).

- **39 (al final):** rutas completas Sora y Reverse/Rebirth. Método aprobado:
  speedrun (.vbm USA en la raíz) como guía por tramos, comparar con el emulador
  en puntos de control (sala, mundo, HP, escena), corregir el desfase por lag,
  replicar y verificar cada escena; Reverse/Rebirth con el bot (KHCOM_BOT).
- **44:** limpieza: hecha la de carpetas ReKHCoM-vX; falta worktrees y carpetas
  build viejas (preguntar antes de borrar).
- **51 (nuevo, 18/10): PS Vita.** Decisión del usuario: de ggdrn/KH_COM_VITA (MIT, v0.08.08)
  solo se toma la FORMA DE COMPILAR (VitaSDK + vitaGL, ROM sin datos en el .vpk con
  tira/auditoría y carga al arrancar desde ux0:data, kubridge opcional); todo lo demás
  es nuestro (PPU, m4a, opciones, trucos, vista amplia, guardados). Fases:
  1) VitaSDK+vitaGL en la VM Debian, compilar su repo como referencia y probar el .vpk en
  Vita3K (Windows); 2) build Vita de nuestro árbol: punteros reubicables en vez de
  direcciones GBA fijas (la Vita no permite mapa fijo), macros de acceso a hardware,
  capa propia de vídeo/audio/entrada (vitaGL, sceAudioOut, sceCtrl); 3) .vpk alpha
  (EU/US/JP) con aviso de ROM propia; validación en Vita real = reportes. Atribución MIT
  en NOTICE si se adapta algún script. Grande (semanas); va después de lo corto.
- **52–54 (18/10, el usuario ordena empezar PS4, Switch y Vita junto con PS5; orden:
  PS5 estable → Vita fase 1 → PS4 → Switch; estimación sin probar):**
  52 **PS4** (el más viable; el guest x32 de port/ps5 es el mismo código, falta host propio con
  sceVideoOut/sceAudioOut/pad dibujando con nuestro PPU por software; probar en shadPS4):
  x86-64 como la PS5; reutilizar el guest de 32 bits y el
  puente host de port/ps5 con otro SDK (OpenOrbis/GoldHEN) y otra salida de vídeo.
  53 **Switch**: aarch64 de 64 bits; exige o bien la pasada de 64 bits del núcleo o un
  proceso de 32 bits (a verificar en libnx/Horizon). 54 **PS3**: Cell PowerPC big-endian
  (el juego asume little-endian en estructuras y datos de ROM), 256 MB: muy improbable.
  La pasada de 64 bits desbloquearía también macOS (hoy solo Web o Wine). Hardware real = reportes.
- **43:** mantener esta cola y los contadores tras cada cierre.

## Cerrados hoy (referencia)

Márgenes sin pintar en las escenas del tutorial (56/56b) en f5ddc75b9, sale en v1.9.6. Menú de Opciones a toda la altura (57) y aviso moguri a Experimental (64), publicados en v1.9.5. 29 no se reproduce (Mapamundi correcto en los 12 mundos a 16:9; la revisión de objetos de sala la cubren los 36/36 menús de 28): cerrado. 36–37 web probada por el usuario en navegador real: va bien. Mando (49) y 057 playa (12) aprobados por el usuario. Guardados y opciones (5–9), #17 borde de Riku (10), #18 Mapamundi (11), 061 hielo
y 074 Monstro (13–14), remapeo y gatillos (25), submenús (28), fuente CJK Linux
(34), PS5 (38), táctil (45), transición de Opciones (50), regresión sincorazones (4).
Publicado en Nightly 5 (18/10, v1.9.3-nightly.20261018.5): #17 borde de Riku, logros locales,
arte del mando (49 aprobado), transición de Opciones, CJK Linux, corrección Linux #19.
Publicados antes: Nightly 4 (trucos, revanchas, aviso moguri, editor de partidas,
#16, diagrama de mando v1). Ver NIGHTLY4_PUBLICACION.md y NIGHTLY5_PUBLICACION.md.

## Futuro muy lejano (no cuenta como pendiente)

- 057 playa (12, aprobada): igualar los tiles del lado superior derecho con el
  izquierdo y que el pasto no tenga motivos cortados. Solo si algún día sobran tokens.

## Esperan reportes (no cuentan como pendientes)

Mando físico, Linux/Steam Deck, Anbernic RG34XX, Android/Odin 2 y móvil real:
se reabren solo si un reporte muestra un fallo.

## Fuentes

LOGICA_APROBADOS.md, PRUEBAS_VISUALES.md, CONTROLADOR_LOGICA.md,
MENU_TRANSICIONES_LOGICA.md, ESCENAS_REVISION_AMPLIA.md, CODEX_TAREAS.md,
CODEX_RESPUESTAS.md, LINUX.md, WEB_PENDIENTES.md. Pruebas e imágenes en build/;
no versionar ROM, saves ni savestates.
