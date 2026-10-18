# Editor de saves y Trucos — ampliación del usuario, 18/10/2026

Responsable: **Claude**, continuación del bloque 30; pestaña launcher en 47.
Codex integra las entregas terminadas y continúa sus correcciones 25–29/36–40/46.
No abrir dos implementaciones del mismo editor. Arte extendido sigue al final.
Lógica técnica detallada: [SAVE_EDITOR_LOGICA.md](SAVE_EDITOR_LOGICA.md).

**Decisión confirmada por el usuario:** conservar formato y límites originales.
Cartas de combate: 999 en total; catálogo/reposición de cartas elegidas, sin
ampliar SRAM ni llenar 99 de cada variante simultáneamente. Mostrar el espacio
disponible y no borrar cartas para hacer sitio. Respetar también los límites
nativos de mapa (9 por entrada y 99 normales en total).

## Encargo explícito

- Pestaña nueva del launcher: editor de saves integrado y presentación cuidada.
- Al activarlo, aviso con confirmación explícita y posibilidad de cancelar.
- Al añadir cartas, mostrar sus sprites originales a escala ×1, sin suavizado.
  Obtenerlos del cartucho instalado como los otros recursos del launcher.
- Permitir identificar archivo, región, banco y ranura; mostrar cambios antes
  de aplicar, conservar copia previa y respetar checksums/cabeceras.
- Pruebas sobre copias: reabrir save modificado en editor y juego, preservar la
  otra ranura, regiones EU/US/JP y valores máximos realmente válidos.

## Trucos pedidos por el feedback del usuario

1. Enemigos derrotados de un golpe.
2. Vida infinita.
3. 99 de todas las cartas de mapa (las utilizadas para abrir puertas).
4. 99 de todas las cartas de combate: llaves espada y sus valores, enemigo/jefe.
5. Nivel máximo **con todos** los bonos de subida: HP máximo, CP máximo y todos
   los sleights. Cambiar solo el número del nivel no satisface el encargo.

La intención expresada es facilitar combate a quien se atasca con las cartas.
No eliminar la mecánica normal ni activar trucos por defecto. Comprobar límites
de inventario y validez por personaje; no trasladar direcciones USA a EU/JP.

## Fuentes que dejó el usuario

- https://github.com/RyudoSynbios/game-tools-collection — referencia del editor.
  Consultada: repositorio público, licencia MIT indicada en GitHub. Verificar
  licencia y estructura del módulo concreto antes de reutilizar código.
- https://gamefaqs.gamespot.com/gba/919011-kingdom-hearts-chain-of-memories/faqs/42470
  — consulta web bloqueada al registrar el encargo; contenido aún sin verificar.
- https://almarsguides.com/retro/walkthroughs/gba/games/kingdomheartschainofmemories/codebreaker/
  — listado CodeBreaker USA consultable, referencia para contrastar comportamiento.
- https://gamehacking.org/?game=5024&hacker=all&format=cb
  — listado USA/CodeBreaker consultable.

Los códigos son referencias: contrastar los campos con las estructuras y lógica
de la decompilación regional antes de implementar. No ejecutar parches externos
ni copiar tablas extensas sin revisar su procedencia/licencia.
