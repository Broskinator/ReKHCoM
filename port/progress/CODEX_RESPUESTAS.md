# Estado de respuestas y cierres de issues

Registro actualizado tras confirmar el estado público en GitHub el 18/10/2026.
Los resultados de pruebas locales están bajo `build/issue-closure-20261018/`.

## Cerrados con respuesta y evidencia

- **#4 — Zoom:** 56 combinaciones de resolución/zoom pasan, incluida 2880×1800.
  Respuesta de cierre: https://github.com/MatyMeatBoy/ReKHCoM/issues/4#issuecomment-6250827053
  Estado: CLOSED. Nightly 3: https://github.com/MatyMeatBoy/ReKHCoM/releases/tag/v1.9.3-nightly.20261018.3
- **#5 — 16:10:** Automatic produce 256×160 a ×5 en 1280×800; regresiones de zoom/aspecto
  56/56. El comentario posterior del usuario comparando otro build pide resolución y
  función exactas para una solicitud separada. Cierre: https://github.com/MatyMeatBoy/ReKHCoM/issues/5#issuecomment-6250827701
  Estado: CLOSED.
- **#6 — dificultad Proud:** multiplicadores confirmados y regresión EU/US/JP; respuesta:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/6#issuecomment-6250828505
  Estado: CLOSED.
- **#12 — selección de guardado:** arreglo de resúmenes, cambio de archivo actual,
  último slot y persistencia; harness EU con tiempos separados 00:00:11/00:00:22;
  EU/US/JP compilan. Respuesta: https://github.com/MatyMeatBoy/ReKHCoM/issues/12#issuecomment-6250829207
  Estado: CLOSED.
- **#13 — gatillos remapeables:** LT/RT virtual en reposo, umbral, individual/simultáneo,
  liberación, overlay y desconexión; compilación EU/US/JP. La respuesta aclara que no
  prueba mandos físicos concretos: https://github.com/MatyMeatBoy/ReKHCoM/issues/13#issuecomment-6250829898
  Estado: CLOSED.
- **#14 — voces japonesas:** dos WAV de ataques Sora comparados EU/US con ROM B8CJ
  verificada; flujo incluido en Nightly 3. Respuesta:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/14#issuecomment-6250830423
  Estado: CLOSED.
- **#16 — selección de cartas de puerta en vistas fijas:** código 4287b8053, 15/15
  pruebas EU/US/JP en cinco vistas con puerta real; comentario técnico:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/16#issuecomment-6251392001
  Estado: CLOSED por Codex el 18/10 después de verificar el comentario y el estado.

## Abiertos / esperando alcance

- **#2 — funciones del remake:** pregunta inicial:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/2#issuecomment-6250834777
  El autor respondió que busca una edición definitiva y enlazó la página de Re:CoM:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/2#issuecomment-6251472356
  La referencia enumera cambios de historia, jefes, mecánicas y contenido; aún no hay
  una función concreta priorizada. Follow-up con ejemplos compatibles con GBA:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/2#issuecomment-6253397772
  Mantener abierto hasta que priorice una o dos opciones.
- **#3 — texto HUD:** el autor confirmó que las actualizaciones lo resolvieron y cerró
  el issue. Comentario de Codex solicitando pantalla/texto/HD si se reproduce:
  https://github.com/MatyMeatBoy/ReKHCoM/issues/3#issuecomment-6250835005
  Estado actual: CLOSED por el autor.

No cerrar un issue solo porque haya un commit: registrar el comentario que explica
la corrección, las pruebas exactas y confirmar el estado de GitHub.
