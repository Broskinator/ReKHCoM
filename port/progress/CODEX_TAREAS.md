# Tareas de Codex — 18/10/2026

Encargo del usuario: Codex hace las issues y mejoras de abajo; **Claude integra**.

## Reglas

- Codex **solo commitea**, nunca hace push ni publica (releases, Pages, issues).
- Trabajar en un worktree nuevo desde `origin/main`, p. ej.
  `git -C khcom worktree add ../khcom-codex -b codex/issues origin/main`,
  con una rama por tarea si conviene (`codex/undub`, `codex/remap`...).
  El trabajo antiguo sin commitear de `khcom` (HUD HD) está respaldado en
  `backups/khcom-hdhud-wip-20261014`; no hace falta tocarlo.
- Cambios del juego bajo `PLATFORM_PC`. Toda mejora opcional: menú de opciones
  del juego (port/src/port_ui.c), launcher (port/decompui/decompui.cpp) y
  kh_com.ini (port/src/port_config.c).
- Probar solo en copias aisladas con `SDL_VIDEO_DRIVER=dummy`
  `SDL_AUDIO_DRIVER=dummy` (sin ventanas en la pantalla del usuario) y las
  variables del harness (KHCOM_MODE, KHCOM_KEYS, KHCOM_OUTPUT_SHOT, KHCOM_FAST).
  Nunca en las carpetas `ReKHCoM-vX` del usuario ni en `build/pc`.
- No subir ROMs, saves, estados ni datos extraídos de la ROM.
- Las respuestas a las issues (en inglés) se dejan redactadas en
  `port/progress/CODEX_RESPUESTAS.md`; Claude las publica al integrar.
- Al terminar cada tarea: compilar EU/US/JP, dejar capturas/evidencia en
  `build/codex-evidence/<tarea>/` y anotar aquí qué está hecho y verificado.

## 1. Issue #14 — Undub (PRIORIDAD)

Petición: compatibilidad con la ROM "Undub" (voces japonesas en la versión
EU/US). El parche de la comunidad está en
`KHCoMPC/Kingdom Hearts - Chain of Memories GBA Undub v1.0a/` (.bps y .xdelta
para (E) y (U)). Aplicarlo implicaría distribuir o pedir una ROM modificada.

Hacer: **reimplementar los mismos cambios** dentro del port a partir de la ROM
japonesa original del usuario (`roms/B8CJ.gba`), sin usar ni distribuir la
ROM parcheada. Analizar qué cambia el parche (decodificar BPS/xdelta contra
la ROM EU/US original y clasificar las regiones: audio de voces, tablas de
sonido, punteros...) y reproducirlo leyendo esos datos de la ROM JP en tiempo
de carga.

- Launcher: opción "Undub (voces japonesas)". Si `roms/B8CJ.gba` no está
  instalada y verificada, la opción aparece en gris (deshabilitada) con un
  texto que explica que hace falta la ROM japonesa.
- Clave en kh_com.ini; desactivada por defecto. Sin la ROM JP no cambia nada.
- Verificar con capturas de audio (KHCOM_WAVDUMP) que las voces son las
  japonesas en EU y US, y que sin la opción todo queda idéntico.
- Respuesta para #14 explicando el soporte y cómo activarlo.

## 2. Issue #13 — Reasignar controles

Pantalla típica de PC para cambiar controles: cualquier acción, tanto los
botones originales (A, B, L, R, START, SELECT, cruceta) como las funciones
nuevas (esquivar, acumular, devolver cartas, estados rápidos, opciones...),
asignable a teclado y a mando, **incluidos los gatillos analógicos (LT/RT)**,
que es lo que pide la issue. Ya existe remapeo en el launcher (kb_*, pad_*);
ampliarlo a gatillos y a todas las acciones, y añadirlo también al menú de
opciones del juego. Detectar conflictos y permitir restaurar valores por
defecto. Respuesta para #13.

## 3. Issue #6 — Texto del modo Orgulloso

Mejorar la descripción de la dificultad Orgulloso (Re:CoM / mejoras PS2) en
el juego y el launcher, en todos los idiomas que la muestren, y redactar para
la issue una explicación detallada de qué cambia ese modo respecto al normal
(leer el código: daño, valores, IA...). Respuesta para #6 y cerrarla.

## 4. Respuestas sin código

- **#5 (vista 16:10):** ya soportado: la vista Automática se adapta a la
  forma de la ventana (1280x800 da un lienzo 16:10 exacto; 256x160 con zoom
  x5). Redactar respuesta (Claude decide si cierra).
- **#4 (más zooms):** ya hay zoom de x1 a x6. Respuesta y cierre.
- **#3 (falta el texto grande del HUD):** no está claro a qué se refiere;
  respuesta cortés pidiendo detalles si vuelve a pasar, y cierre.

## 5. Otras mejoras pedidas por el usuario

- Selector del menú de opciones con el efecto del menú de depuración oficial
  (en GBA se entra con CodeBreaker EU `320348DC 0006`, US/JP `3203493C 0006`,
  escritura de 0x06 en esa dirección de EWRAM).
- Trucos: primera opción "Entrar al modo debug" (lo mismo que esos códigos).
- Revisar la implementación de devolver cartas acumuladas (unstock, de
  ggdrn/KH_COM_VITA) y poner estas mejoras en un título propio dentro de las
  mejoras PS2, como el de Experimental.
- Issue #12: elegir la ranura de guardado dentro del juego (opción).
- Arenas tamaño GBA (Guard Armor, Dragon Maleficent): en la vista ancha asoma
  una tira del fondo en el borde derecho; corregir.

## 6. Investigación

- RetroAchievements: ver si el port puede usar los logros de la versión GBA
  (variante/hack o al menos en local con rcheevos) y si las direcciones de
  memoria de los logros coinciden con la memoria GBA emulada del port.
  Informe en español; no contactar ni publicar nada.

## Fuera de este encargo

- Fondos de escenas en 16:9 (extensión con tiles): lo lleva Claude con el usuario.
- Linux: hecho por otra sesión (main e2117b264), entra en la próxima release.
