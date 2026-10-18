# Lógica de continuación — pendientes Codex

Este documento es la guía de trabajo junto a la cola única
[`PENDIENTES_VIGENTES.md`](PENDIENTES_VIGENTES.md). Cada bloque indica el método,
la evidencia necesaria y el criterio para cambiar su estado. No sustituye código,
capturas ni cierres verificables. Editor/cheats: [`SAVE_EDITOR_LOGICA.md`](SAVE_EDITOR_LOGICA.md).
Controles: [`CONTROLADOR_LOGICA.md`](CONTROLADOR_LOGICA.md).

## 25 — Remapeo y gatillos

- Lógica: traducir entradas SDL a acciones del juego sin confundir índice/signo
  de eje; LT/RT analógicos parten en reposo neutral, cruzan umbral, se liberan y
  no repiten en un solo ciclo. Persistir el binding con el formato actual.
- Verificar harness SDL virtual en EU/US/JP, overlay abierto/cerrado, pérdida y
  reconexión; después probar un mando físico y registrar modelo/SDL/reportes.
- Cerrar lo físico cuando se pueda nombrar dispositivo y resultado. La prueba
  virtual no se presenta como cobertura física. Avance anterior: 3fd15b641.

## 26 — Issues #2 y #3

- Lógica: no implementar una petición amplia hasta identificar la función de
  historia o el elemento HUD. Mantener en GitHub una pregunta concreta, revisar
  respuesta nueva y asociar captura/versión si describen un fallo.
- Verificar estado público, contestar con límites y pasos reproducibles; solo
  después crear alcance técnico/prueba. Cierre si el autor aclara, se implementa
  y comprueba, o confirma que ya no necesita el cambio. Hasta entonces abierto.

## 28 — Menús de juego

- Lógica: configurar únicamente el estado inicial con el fixture `room`; congelar
  apariciones enemigas antes del input. Ejecutar una ruta natural de START,
  DOWN, A por proceso y no inyectar otra sala después de entrar o salir del menú.
  Un encuentro aleatorio o fallo del harness no prueba un fallo de menú.
- Para cada submenú comprobar entrada exacta en la traza, navegación/cambio de
  pestaña con controles naturales, regreso a `Mode_MapFld`, proceso sin crash y
  capturas antes/dentro/después. Riku requiere avanzar el tutorial inicial de su
  baraja con botones; luego abre y cierra la vista de baraja fija.
- Matriz de cierre: EU/US/JP, Sora y Riku, las seis opciones, transiciones de
  entrada/salida, capturas y GBA intacta. Builds, capturas y JSON quedan en
  `build/` ignorado. No cambiar mecánica si falla el harness.
- Hallazgo: el harness anterior ejecutaba seis estados en un proceso, forzaba una
  sala sobre una batalla aleatoria y convertía el crash posterior en 0/6 aunque
  Barajas ya había vuelto al campo. También excluía por error Mundo/Diario de
  Riku; al intentar ambas rutas naturales sí entraron y regresaron.
- Cierre: builds frescas EU/US/JP; Sora 6/6 y Riku 6/6 por región, 36/36. Cada
  caso tiene traza, capturas de pantalla y JSON con salida/entrada bajo
  `build/codex-gameplay-menus/`. No se cambió lógica del juego ni se inyectó
  estado durante los menús.

## 29 — Mapamundi y objetos de sala

- Lógica: inventariar desde la auditoría histórica los floors con una sola variante
  que omite suelo/objeto; distinguir variante omitida por diseño de asset faltante.
  Seguir el patrón nativo de las variantes adyacentes, como hizo Monstro: usar el
  tile complementario que cierra el motivo a ambos lados de la unión.
- Para persistencia de objetos, trazar dueño y vida del objeto en cambio de room,
  mapa y recarga; no duplicar una entidad que ya guarda estado en floor. Primero
  hacer fixtures con transiciones concretas y comparar presencia/posición antes,
  después y tras volver.
- Cerrar por lista de variantes y transiciones con prueba en EU/US/JP, capturas
  enteras a zoom nativo/amplio, juego original sin cambios y evidencia de ida/vuelta.

## 36 — Web regional y SRAM

- Lógica: matriz EU/US/JP usando el mismo release web y ROM local de cada región.
  Cubrir arranque, idioma, diálogo, combate, reset y cierre abrupto; para SRAM
  importar una copia válida, guardar en ambos slots, exportar y comparar checksum,
  reiniciar y reimportar. No incluir ROM/save del usuario en el repositorio.
- Ejecutar en navegador real y registrar versión del navegador, SO, consola,
  resultado y capturas/logs sin datos de partida. Fallos del emulador no se
  atribuyen al transporte de archivos sin comparar bytes/checksum.
- Cerrar cada región solo con flujo completo de ida/vuelta y datos idénticos
  fuera de los campos modificados.

## 37 — Web: render, audio, memoria, mando y móvil

- Separar mediciones: FPS y memoria tras arranque/reposo/combate; texto y márgenes
  en ventanas pequeñas y anchas; audio al perder/recuperar foco; teclado/mando;
  interfaz táctil en teléfono/tablet real. Guardar métricas reproducibles.
- Solo probar Worker/OffscreenCanvas si un perfil atribuye el cuello a render o
  bloqueo de hilo. Comparar misma máquina, navegador, escena y duración antes y
  después, junto con compatibilidad de entrada/audio. No proponerlo por intuición.
- Cerrar cada subcaso por separado; móvil real no se infiere de navegador desktop.

## 40 — PSG en combate

- Lógica: fijar ROM/región, encuentro, semilla/acciones, frecuencia de muestreo y
  estado de audio; grabar referencia de juego original y build actual desde la
  misma ruta. Alinear comienzo, separar música/efectos si la herramienta permite,
  y comparar tempo, notas, envolvente, canales, cortes y ruido.
- Reproducir el mismo patrón dos veces para descartar aleatoriedad. Identificar
  driver/canal PSG y reproducir una sola hipótesis de arreglo por vez; validar que
  música de Coliseo, ya resuelta, no vuelva a anotarse como pendiente.
- Cierre: discrepancia localizada y corregida o explicación de incompatibilidad
  aceptada, WAV/medidas reproducibles, combate no PSG sin regresión y compilación
  EU/US/JP.
