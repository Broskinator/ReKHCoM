# Tareas de Codex — 18/10/2026

Encargo del usuario: Codex hace las issues y mejoras de abajo y puede integrarlas en main; Claude prepara las releases.

## Reglas

- **Actualización 18/10 (usuario): Codex también puede integrar.** Puede
  llevar su propio trabajo a `main`: `git fetch`, rebase sobre `origin/main`,
  compilar EU/US/JP sin errores, probar lo tocado y hacer push a `main`.
  No reescribir historia ajena ni forzar pushes. Si `main` trae trabajo de
  Claude en los mismos archivos, integrar con cuidado y avisar en este archivo.
- Codex puede responder issues cuando la implementación y las pruebas estén
  comprobadas (autorización del usuario, 18/10/2026); no cerrar issues sin que
  el usuario lo pida. Claude sigue preparando releases y actualizando Pages.
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
- Las respuestas a las issues (en inglés) se registran en
  `port/progress/CODEX_RESPUESTAS.md`; Codex puede publicar las que ya tengan
  cambios y pruebas comprobadas. No cerrar issues sin indicación del usuario.
- Al terminar cada tarea: compilar EU/US/JP, dejar capturas/evidencia en
  `build/codex-evidence/<tarea>/` y anotar aquí qué está hecho y verificado.

## Estado de esta repartija — 18/10/2026

- **Hecho y probado:** análisis de BPS del Undub sin generar ni guardar ROM
  modificada; capa opcional de voces JP desde B8CJ validada, por región y OFF
  por defecto. `python port/tools/test_undub.py` captura dos voces de Sora en
  EU/US: los dumps cambian con Undub, coinciden entre regiones y permanecen
  regionales cuando se apaga. Evidencia en `build/codex-evidence/undub/runs/`.
- **Hecho:** remapeo de controles de juego en launcher y menú, incluidos ejes
  de gatillos, las acciones de esquivar/acumular/devolver, restauración de
  defaults y aviso rojo de asignaciones repetidas en el launcher. EU/US/JP
  compilan; launcher pasa 11/11 pruebas. #13 queda listo para revisión manual
  con mando.
- **Hecho:** texto Orgulloso/Beginner actualizado. #12 (selección de ranura en
  puntos de guardado) ahora está detrás de Opciones > Experimental, apagada por
  defecto y probada por el harness de menú/configuración. Se prepara para una
  nightly pública; la selección/escritura de ambas ranuras aún requiere prueba
  manual de gameplay antes de una release estable. Sigue siendo opcional y de
  baja prioridad.
- **Hecho:** selector de opciones con estilo debug, acceso de modo debug bajo
  Trucos y grupo propio para las mejoras Re:CoM. Unstock continúa opcional y
  apagado por defecto.
- **Respuestas:** #4, #5, #6 y #14 publicadas tras validar las pruebas; #12 y
  #13 siguen sin respuesta hasta completar la validación correspondiente. Los
  borradores de preguntas #2 y #3 quedan en `CODEX_RESPUESTAS.md`.
- **Pendiente:** corregir la tira de fondo en las arenas GBA y verificarla.
- **RetroAchievements:** investigación en `RETROACHIEVEMENTS.md`. Confirmados
  set base (90) y `Subset - Level 1` (39, ROM parchada con hash distinto), y
  RaceWave46 como referencia pública GPL-3.0. Esta rama aún no integra el
  cliente, la sesión online ni los popups; no anunciar los sets como jugables.
  El siguiente bloque debe resolver `rc_client`/licencia/HTTP/ROM IDs y luego
  implementar y probar notificaciones para ambos sets, no solo el diseño.
- **Pruebas verificadas:** build Windows EU/US/JP; launcher 11/11; Re:CoM
  80 combinaciones; zoom 41 casos; aspecto 56 combinaciones. La prueba de
  opciones del juego se interrumpió por timeout y no cuenta como aprobada.

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
- Verificado con `port/tools/test_undub.py`: WAV dumps de dos voces de Sora en
  EU/US, modo OFF idéntico entre regiones, modo ON distinto al OFF e idéntico
  entre EU/US. El test exige además que el log confirme la carga del mapa JP.
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

- **#5 (vista 16:10):** la vista Automática se adapta a la forma de la ventana;
  prueba exacta: 1280x800 da un lienzo de 256x160 a ×5. Respuesta publicada;
  el issue sigue abierto.
- **#4 (más zooms):** selector dinámico por resolución; en 2880x1800 llega a
  ×11. 56 casos pasan. Respuesta publicada; el issue sigue abierto.
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
- Issue #12 (baja prioridad/opcional): elegir la ranura de guardado dentro del
  juego. Si se integra en una release, debe activarse desde Opciones >
  Experimental; no dejar la selección de ranura siempre activa.
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
