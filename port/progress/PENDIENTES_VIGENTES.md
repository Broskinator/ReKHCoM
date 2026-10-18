# Lista única larga y reparto vigente — 18/10/2026

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

Los responsables indicados son asignaciones para continuar DESPUÉS de recoger
los bloques actuales; no son afirmaciones de que se hayan iniciado más workers.
[x] cerrado; [~] implementado/en curso; [ ] pendiente. No hay aprobación visual
final de 057/061/074: solo 004 León permanece aprobada (1/44).

1. [x] **Codex/coordinación:** cierre recibido de «Revisar notas de Claude»;
   main a1d4147e2, sin workers propios pendientes y Darkside WIP preservado.
2. [~] **Codex/coordinación:** recoger cierre del chat del bug de sincorazones y
   el inventario de sus workers. Conservar resultado y pendientes por separado.
3. [ ] **Claude/coordinación:** terminar/recoger sus workers externos, registrar
   cuáles acabaron y qué dejó cada uno. No iniciar otra tanda antes de consolidar.
4. [ ] **Codex:** verificar el cierre del bug grave de sincorazones pequeños
   que desaparecen sin animación, usando el savestate del usuario y el resultado
   del chat actual; continuar su corrección si el cierre resulta incompleto.
5. [x] **Claude:** resumen al cambiar de slot, incluida ranura vacía; e9f053595,
   integrado aa8ef4434. Informe CLAUDE_CIERRE_20261018.md.
6. [x] **Claude:** harness EU mapsave comprueba guardado en ambas ranuras,
   aislamiento de resúmenes (00:00:11 / 00:00:22) y reapertura en la segunda.
   EU/US/JP compilan. Evidencia de Claude en CLAUDE_CIERRE_20261018.md.
7. [x] **Claude:** persistencia del selector implementada en e9f053595;
   launcher registra la clave y conserva también claves desconocidas.
8. [ ] **Claude:** auditar persistencia de las otras opciones reportadas y mantener
   el selector de slots dentro de Experimental.
9. [x] **Claude:** último slot corregido en e9f053595 mediante la bandera nativa
   guardada en la partida. Comprobación conjunta sigue en el punto 6.
10. [ ] **Claude:** localizar y respaldar el savestate donde el mapa queda abierto
    arriba; reconstruir cierre con sus propios tiles de borde.
11. [ ] **Claude:** savestate 3, Hollow Bastion: completar checkerboard lateral,
    conservar fase, título/cartel, iconos y objetos.
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
25. [~] **Codex:** remapeo/gatillos implementado; LT/RT comprobados con SDL virtual
    (reposo, umbral, simultáneos, liberación, cierre de overlay y desconexión).
    EU/US/JP compilan; test_web_controls.py PASS. Falta mando físico, sin
    atribuirle a la prueba virtual validación de dispositivos concretos.
26. [ ] **Codex:** issues #2/#3: concretar la función de historia y elemento HUD
    reportados antes de implementar; no prometer un remake completo.
27. [ ] **Codex:** RetroAchievements: pasar de investigación a implementación y
    comprobación del set base/Level 1; investigación actual no equivale a soporte jugable.
28. [ ] **Codex:** cerrar revisión de submenús Diario/Estado/Barajas/Naipes y
    transiciones; conservar lo ya resuelto y verificar variantes pendientes.
29. [ ] **Codex:** revisar variantes de planta de Mapamundi y conservación de
    objetos de sala en transiciones, según PENDIENTES.md histórico.
30. [ ] **Claude:** cheats pedidos expresamente: enemigos de un golpe, vida
    infinita, 99 de cada carta de mapa y de combate (valores de llaves espada,
    cartas enemigo/jefe), nivel máximo con todos los bonos de HP, CP y sleights.
    Mostrar controles claros en pestaña Trucos; fuentes y alcance en
    SAVE_EDITOR_Y_CHEATS.md. No confundir nivel máximo con sus bonos completos.
31. [ ] **Claude:** mejoras Vita pendientes registradas: aviso de puntos moguri,
    revancha de jefes y repetición de tutoriales; conservar como cola posterior.
32. [ ] **Claude:** Linux real y Steam Deck: X11/XWayland, sonido, controles y
    rendimiento. Hay pruebas VM; no repetir la afirmación antigua de «sin binario».
33. [ ] **Claude:** Anbernic RG34XX real: kernel/32 bits, framebuffer, orientación,
    mando/PortMaster y rendimiento; la VM ARM no mide rendimiento físico.
34. [ ] **Claude:** Linux, fuente CJK para texto HD japonés y launcher gráfico
    pendiente; conservar ajustes del menú del juego mientras tanto.
35. [ ] **Claude:** Android físico: guardados, controles táctiles, audio, memoria,
    orientación y rendimiento; mantener alpha donde falte validación real.
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
39. [ ] **Claude:** rutas completas Sora y Reverse/Rebirth, campañas/regiones;
    baterías cortas sin crash no equivalen a partidas completas.
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
44. [ ] **Claude:** limpieza de worktrees/builds antiguos después de verificar
    snapshots y condiciones previas; nunca borrar carpetas ReKHCoM-vX del usuario.

45. [x] **Codex:** Ocultar controles táctiles en Experimental, Android/Web;
    preferencia persistente, limpieza de pulsaciones y acceso a Opciones.
    6d0f33df1; pruebas de entrada, persistencia y controles Web correctas.
    Odin 2 y otros dispositivos físicos pendientes de validar (35/37).

46. [~] **Codex:** issue #16, pantalla negra al sintetizar puertas fuera de vista
    Automática. Reproducir con entrada real de puerta y comprobar todas las vistas;
    prioridad sobre extensiones de arte. Claude mantiene #17/#18 (10–11).

47. [ ] **Claude (ampliación del bloque 30 por el usuario):** pestaña nueva del
    launcher para editor de saves basado en game-tools-collection, con aviso y
    confirmación al activarla; cartas con sprites nativos ×1 y presentación cuidada.
    Archivo/región/banco/ranura explícitos, vista previa, copia de seguridad y
    escritura válida comprobada. Requisitos/fuentes: SAVE_EDITOR_Y_CHEATS.md.

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
