# Lista única larga y reparto vigente — 18/10/2026

**Resumen 18/10 17:10:** 44 puntos: 20 cerrados, 8 en curso, 16 abiertos. Hardware real → «Esperan reportes». Integrado de Codex (EU/US/JP compilan): logros locales, mando en Controles, transición de Opciones.
Siguiente de Claude: 39 (rutas completas) y 44 (limpieza, con permiso); el resto es de Codex. Lógica aprobada: LOGICA_APROBADOS.md.

Encargo del usuario: TERMINAR EL BLOQUE DE LOS WORKERS, RECOGER SUS CIERRES y
continuar desde una sola cola larga repartida entre Claude y Codex. No abrir
nuevos workers mientras se cierra esta tanda. Publicación autorizada al terminar
la entrega corregida. Esta lista sustituye los repartos anteriores; los demás
documentos quedan como evidencia técnica e historial, no como colas paralelas.

## Cierre de workers: acciones reales

- Este chat: inventario consultado; cero subagentes activos, solo agente principal.
- Chat «Revisar notas de Claude»: pedido de cierre enviado mediante herramienta.
  CIERRE CONFIRMADO: main a1d4147e2; sin subagentes propios pendientes.
  Darkside incompleto preservado en wip/darkside-backdrop-20261018, commit
  6e7400189; excluido de releases. Informe en CIERRE_WORKERS.md.
- Chat del bug de sincorazones: pedido de terminar bloque y recoger workers enviado.
  La herramienta aceptó el mensaje; lectura de sus turnos no soportada. Cierre sin confirmar.
- No confundir detener un proceso con acabar el trabajo. Recoger commit, pruebas,
  pendientes y WIP recuperable de cada uno; no interrumpir una escritura a mitad.
- Los cierres deben quedar en CIERRE_WORKERS.md. Claude coordina sus propios
  workers externos; este chat no puede certificar su cierre sin recibir el informe.

## Orden y propiedad del trabajo (corrección del usuario)

- **Continuidad recíproca (nueva instrucción del usuario):** Claude y
  Codex/ChatGPT pueden seguir commiteando/pusheando y tomar pendientes del otro
  tras 10 minutos sin nuevos commits ni actividad verificable. Revisar también
  cambios de archivos y builds/pruebas activos; anotar hora/evidencia y transferir
  el bloque aquí antes de editar. Preservar WIP, no duplicarlo. Al volver, leer
  las asignaciones actualizadas. Esta regla flexibiliza las asignaciones de abajo.
- **Claude conserva implementación de guardados 5–9 y savestates 10–11.**
  Guardados implementados en e9f053595 e integrados como aa8ef4434; no rehacerlos.
  El usuario transfirió a Codex integración/publicación 41–42 de esta prerelease.
- **Codex toma primero sus pendientes cortos 25–26, 28–29, 36–37 y 40**, según
  dependencias y evidencia disponible; RetroAchievements queda después.
- **Extensiones de arte 12–24 al final**, después de bugs y funciones.
- Antes de editar: comprobar propietario en esta lista y commits/estado del
  checkout del propietario, no solo main. Un único responsable de implementación
  por tarea. Transferirla expresamente antes de cambiar de responsable.
- Codex retiró su implementación duplicada de guardados del árbol compartido.
  Conservada solo como borrador privado en build/duplicate-save-draft;
  NO integrar, NO publicar. La implementación válida es la de Claude.

## Cola de trabajo, numeración única

Cada pendiente debe enlazar su lógica de implementación, comprobaciones y criterio
de cierre. Mantener LOGICA_PENDIENTES_20261018.md al añadir o concretar tareas.
Para 30/47: SAVE_EDITOR_LOGICA.md; para 48: CONTROLADOR_LOGICA.md.

Los responsables indicados son asignaciones para continuar DESPUÉS de recoger
los bloques actuales; no son afirmaciones de que se hayan iniciado más workers.
[x] cerrado; [~] implementado/en curso; [ ] pendiente. No hay aprobación visual
final de 057/061/074: solo 004 León permanece aprobada (1/44).

1. [x] **Codex/coordinación:** cierre recibido de «Revisar notas de Claude»;
   main a1d4147e2, sin workers propios pendientes y Darkside WIP preservado.
2. [~] **Codex/coordinación:** recoger cierre del chat del bug de sincorazones y
   el inventario de sus workers. Conservar resultado y pendientes por separado.
3. [x] **Claude/coordinación:** Claude no lanzó workers externos en esta tanda;
   nada que recoger (18/10).
4. [x] **Codex:** regresión compilada desde macros y funciones de producción;
   límites ±1, entrada/salida y `y + z` pasan en GBA/16:9/32:9.
   Copia de `slot1.khst` corre en su build EU compatible y el Heartless aparece
   y se mueve sin desaparición; main rechaza esa state por build ID distinto.
   El defecto exacto no se reprodujo con este estado, así que reabrir si llega
   un estado/captura que lo demuestre. Detalle en LOGICA_PENDIENTES_20261018.md, §4.
5. [x] **Claude:** resumen al cambiar de slot, incluida ranura vacía; e9f053595,
   integrado aa8ef4434. Informe CLAUDE_CIERRE_20261018.md.
6. [x] **Claude:** harness EU mapsave comprueba guardado en ambas ranuras,
   aislamiento de resúmenes (00:00:11 / 00:00:22) y reapertura en la segunda.
   EU/US/JP compilan. Evidencia de Claude en CLAUDE_CIERRE_20261018.md.
7. [x] **Claude:** persistencia del selector implementada en e9f053595;
   launcher registra la clave y conserva también claves desconocidas.

9. [x] **Claude:** último slot corregido en e9f053595 mediante la bandera nativa
   guardada en la partida. Comprobación conjunta sigue en el punto 6.
10. [x] **Claude:** #17. Borde superior de la sala de Riku: el borde inferior
    festoneado (tira 24x4) se repite volteado arriba sobre muro liso.
    APROBADO TOTALMENTE por el usuario (18/10); activo por defecto (KHCOM_TOPCLOSE=0
    lo apaga). Lógica en LOGICA_APROBADOS.md.
11. [x] **Claude:** #18 Mapamundi: el usuario confirma que ya quedó arreglado (18/10).
12. [~] **Codex:** 057 playa corregida para vistas amplias; código y capturas listos,
    falta revisión visual final de la nueva extensión.
13. [~] **Codex:** 061 hielo corregida para vistas amplias; código y capturas listos,
    falta revisión visual final de la nueva extensión.
14. [~] **Codex:** 074 Monstro corregida para vistas amplias; motivos completos,
    código y capturas listos, falta revisión visual final de la nueva extensión.
15. [x] **Codex:** entregar 057/061/074 en main 0cb04a8dd; fuente 643739f9b.
    EU/US/JP compilan; 23/23 regionales y 6/6 tras último ajuste Monstro;
    capturas reales 960×540/1440×810, GBA intacta y León idéntica.
16. [ ] **Codex:** retomar Darkside desde 6e7400189, rama
    wip/darkside-backdrop-20261018. Cierre recibido: quedan recorte rectangular
    del cuerpo y energía repetida en ultrawide; faltan regresiones y EU/US/JP.
    Mantener este WIP fuera de main y releases hasta corregirlo.
17. [ ] **Codex:** Oogie Boogie: continuar sala sin duplicar ruleta ni rejas.
18. [ ] **Codex:** Jafar: continuar lava/pared respetando pilares y fases móviles.
19. [ ] **Codex:** Maléfica dragón: continuar salón sin repetir al dragón.
20. [ ] **Codex:** Úrsula: continuar caverna sin duplicar a la jefa.
21. [ ] **Codex:** Marluxia final: corregir plataforma y capas del jefe cortadas/repetidas.
22. [ ] **Codex:** Alicia/Trickmaster: solo completar tablero inferior pendiente.
    Guard Armor ya aceptado e integrado: 1/8. Hook/Monstro de combate se conservan.
23. [ ] **Codex:** continuar resto de candidatas de escenas, una a una, después
    de revisar estas tres; antes/después completo y contador. No rellenar vacíos diseñados.
24. [ ] **Codex:** revisar tira de fondo de arenas GBA; distinguir lo que ya corrige
    Guard Armor de los casos que aún quedan, en especial Maléfica dragón.
25. [x] **Codex:** remapeo/gatillos implementado y comprobado con SDL virtual;
    EU/US/JP compilan. Mando físico: llegará como reporte (ver abajo).
    18/10: usuario conecta 8BitDo Ultimate 2C por dongle; SDL3 detecta
    XInput Controller #1, VID 2DC8 / PID 310A. Informe físico recibido:
    15/15 botones pulsados/liberados, 8/8 direcciones de sticks, 4/4 ejes
    centrados, LT/RT pulsados/liberados y simultáneos; SDL virtual=0.
    Reconexión y persistencia de remapeo físicas no incluidas en esta prueba.
    Launcher: diagnóstico con botones/ejes/gatillos en vivo y exportación de
    observaciones a controller_report.txt; apertura de mando corregida también
    fuera de captura de remapeo. No confundir ese panel con una prueba física hecha.
26. [~] **Codex:** #3 cerrado por su autor tras confirmar que se corrigió. #2
    sigue abierto: el autor pide una edición definitiva y enlazó Re:CoM; se le
    pidieron una o dos prioridades GBA concretas en el comentario
    6253397772. No prometer un remake completo mientras falta priorización.
27. [~] **Codex:** rcheevos 12.5.0 integrado en Windows; pestaña blanca de logros,
    cuenta/catálogo experimental, token DPAPI y HTTPS asíncrono. 16 retos locales
    propios del port para EU/US/JP, progreso por región, avisos animados centrados
    arriba y prueba de vista previa. test_achievements.py PASS (puente nativo por
    región y contrato con servidor sintético); builds EU/US/JP pasan. Falta login
    real y validar la proyección de globals/punteros nativos contra el set oficial;
    por ello el catálogo oficial no evalúa ni envía premios. Level 1 sigue pendiente.
28. [x] **Codex:** revisión de submenús y transiciones completada en
    LOGICA_PENDIENTES_20261018.md: builds EU/US/JP; Sora 6/6 y Riku 6/6 por
    región (36/36). Incluye tutorial inicial de baraja Riku, capturas y regreso
    al campo. El harness aísla cada menú para que un encuentro no contamine otro.
29. [ ] **Codex:** revisar variantes de planta de Mapamundi y conservación de
    objetos de sala en transiciones, según PENDIENTES.md histórico.

32. [x] **Claude:** Linux/Steam Deck: binario y pruebas VM hechos; validación real
    llegará como reporte.

33. [x] **Claude:** Anbernic: paquete alpha publicado; validación en RG34XX real
    llegará como reporte.

34. [x] **Claude:** fuente CJK de Linux: búsqueda ampliada (KHCOM_CJK_FONT, rutas
    Noto/Takao/IPA/Droid/WQY y fc-match), compila; confirmación en Linux real
    llegará como reporte. Launcher gráfico Linux: fuera de la cola.

35. [x] **Claude:** Android: alpha publicada; validación en dispositivos físicos
    (p. ej. Odin 2) llegará como reporte.

36. [ ] **Codex:** Web EU/US/JP en navegador real: arranque, diálogos, combate,
    importación/exportación SRAM, persistencia, reset y cierre abrupto.
37. [ ] **Codex:** Web: texto HD/menús, audio/foco, FPS/memoria, controles y
    móvil real. Evaluar Worker/OffscreenCanvas solo si mejora medida; es distinto
    de los workers/agentes cuyo cierre pide ahora el usuario.
38. [x] **Worker PS5:** bloque de implementación entregado e integrado.
    PR #15 fusionado en main (5a068ce82), integración de build/release en
    13fba3337 y handoff en 66f4e8c54. Usuario confirma cierre del worker el
    18/10/2026 y pide incluir PS5 en futuras releases. No reabrir este worker
    para rehacer el port. Las comprobaciones de cada paquete y la validación
    física pendiente se conservan como parte de publicación/plataforma, no
    como implementación aún sin entregar. Detalle: CIERRE_WORKERS.md.
39. [ ] **Claude (AL FINAL, aprobado el plan 18/10):** rutas completas Sora y
    Reverse/Rebirth. Método elegido por el usuario: usar el speedrun (.vbm USA en
    la raíz) como guía, no como reproducción exacta: reproducir por tramos,
    comparar con el emulador en puntos de control (sala, mundo, HP, escena) y
    corregir el desfase por los fotogramas de lag; replicar y verificar cada
    escena frente al emulador. Reverse/Rebirth sin movie: bot (KHCOM_BOT).
    Baterías cortas sin crash no equivalen a partidas completas.
40. [ ] **Codex:** comparación de audio PSG pendiente frente a referencia de
    combate; no repetir como pendiente la música de Coliseo ya explicada.
41. [x] **Codex (transferido por el usuario para esta prerelease):** integrados
    guardados e informe de Claude (aa8ef4434, b7e9da465), táctil 6d0f33df1 y
    build Linux/Web 333b0f9a0. Compilación EU/US/JP; sin duplicar PPU ni guardados.
42. [x] **Codex (transferido por el usuario para esta prerelease):** publicada
    v1.9.3-nightly.20261018.3 con Windows, Linux, Steam Deck, Anbernic alpha,
    Android alpha, Web alpha y PPSA98511.zip PS5 alpha recién compilado.
    Pages actualizado: manifiesto y hashes de sus 19 archivos públicos comprobados.
    Stable sigue v1.9.2. URL, hashes, fuentes y límites: NIGHTLY3_PUBLICACION.md.
43. [ ] **Claude + Codex:** actualizar contadores/status/documentos tras cada
    cierre; conservar solo esta cola como vigente y eliminar contradicciones históricas.
44. [~] **Claude:** limpieza. Hecho 18/10 (autorizado): borradas las carpetas
    ReKHCoM-vX antiguas y sus .bat; quedan v1.9.1, nightly.20261018, .2 y la copia
    de prueba de Riku; guardados/ini respaldados en backups/saves-releases-borradas-20261018.
    Pendiente: worktrees y carpetas build viejos (preguntar antes de borrar).
45. [x] **Codex:** Ocultar controles táctiles en Experimental, Android/Web;
    preferencia persistente, limpieza de pulsaciones y acceso a Opciones.
    6d0f33df1; pruebas de entrada, persistencia y controles Web correctas.
    Odin 2 y otros dispositivos físicos pendientes de validar (35/37).

49. [~] **Codex:** corrección de ubicación del mando solicitada por el usuario:
    diagrama Ultimate 2C con acabado toon en Controles del juego, debajo de la
    lista, nombres Xbox y resaltado del binding seleccionado. Builds EU/US/JP;
    capturas nativas 1080p/1016x570/480x320. Aprobación visual pendiente.
    Arte Gemini elegido después por el usuario: integrado con transparencia real,
    máscaras de botones/cruceta/sticks y pestañas LT/LB/RB/RT. Capturas de A, LB,
    LT y LS←; copia jugable «Probar menu y mando.bat» en la raíz. Aprobación visual
    pendiente. Lógica y procedencia en CONTROLADOR_LOGICA.md y assets/controller.

50. [x] **Codex:** transición de opciones sin salto de escala: conservar lienzo y
    presentación del juego hasta negro, luego cambiar al menú; fundido final sobre
    todas las capas. Luz original del debug, secuencia/colores de ROM y reinicio al
    mover selección, sin cambiar medidas de cuadros/textos. Builds EU/US/JP y
    capturas/vídeo 1080p de entrada/salida desde título, idiomas y campo; 56 casos
    de regresión de zoom pasan. Detalle: MENU_TRANSICIONES_LOGICA.md.

## Esperan reportes (no cuentan como pendientes)

Validación en hardware real (mando físico, Linux/Steam Deck, Anbernic RG34XX,
Android/Odin 2, móvil real): el usuario las trae cuando lleguen reportes; solo
se reabre un punto si un reporte demuestra un fallo.

## Publicado en Nightly 4 (18/10, v1.9.3-nightly.20261018.4)

Fuera de la cola: 8 persistencia de opciones, 30 trucos, 31 mejoras Vita
(aviso moguri, revancha de jefes y tutoriales), 46 issue #16, 47 editor de
partidas del launcher, 48 diagrama de mando. Detalle: NIGHTLY4_PUBLICACION.md.

## Qué bloquea la entrega inmediata

Bug grave si sigue abierto (4), guardados/opciones (5–9), savestates (10–11),
revisión final de escenas (12–14), integración y publicación (41–42).
La cola general no transforma por sí sola RetroAchievements o nuevas plataformas
en requisitos de esta entrega. No marcar estos pendientes de largo plazo resueltos.

## Fuentes y resultados

REPARTIJA_20261018.md (encargos), ESCENAS_REVISION_AMPLIA.md (entrega Codex),
CODEX_TAREAS.md / CODEX_RESPUESTAS.md (issues), BATTLE_SCENERY_AUDIT.md,
LINUX.md, WEB_PENDIENTES.md y PENDIENTES.md (historial técnico).
Pruebas e imágenes privadas en build/; no versionar ROM, saves ni savestates.
