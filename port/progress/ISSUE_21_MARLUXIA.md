# GitHub issue #21 — vista de Marluxia 2

19/10/2026, Codex. Para integrar en el próximo release de Claude.

## Causa

El truco de todas las cartas aceptaba el kind 95 (CARD_KIND_MARLUXIA_2),
una carta interna del jefe. gCardKindDescriptions[95] es NULL en los tres
cartuchos. Sora y Riku pasaban ese puntero a LANGSTR/LoadTextSlots sin validarlo;
en EU el propio LANGSTR ya desreferenciaba NULL. Intercambio tenía el mismo acceso.
No es un defecto del dibujo ni de la carta de enemigo normal de Marluxia (kind 94).

## Solución

- El truco excluye el kind 95; conserva la carta normal de Marluxia, los límites
  99 por carta/999 total y las cartas existentes en las barajas.
- Editor nativo y web ofrecen solo kinds de jugador 0..97, excluyendo 95.
- Las tres vistas comprueban el rango del kind y el puntero antes de leer o
  localizar la descripción. Carta interna sin texto: descripción vacía.
  No se borran cartas de guardados existentes ni se inventa una descripción.

## Evidencia

- python port/tools/check_issue21.py: cuerpos de las funciones de producción,
  metadatos reales de las 950 cartas y disponibilidad original de descripciones.
  Ejecuta las tres vistas por carta en EU/US/JP: 8.550 llamadas; excluye las
  11 definiciones del kind 95 (ID 645 y variantes internas) y mantiene 584
  definiciones elegibles. Comprueba que no se llama al lector de texto si falta.
- Compilación nativa completa EU/US/JP: build/issue21-build.log.
- node --check port/web/editor-ui.js: sintaxis válida.
- No se abrió ni modificó ningún guardado del usuario. Estos son controles
  dirigidos y compilación; no se presenta como reproducción visual de una
  partida en hardware. No activa funcionalidad de cartas de jefe.

El arreglo está en código para el siguiente release; no se afirma que las
versiones ya publicadas estén corregidas.
