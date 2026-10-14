# Bugs originales y preservación

Catálogo inicial, 13 de octubre de 2026. No equivale a una lista de errores
confirmados en hardware. Cada caso debe separar cartucho, emulador y port.

## Criterio de trabajo

- Conservar los comportamientos de combate que afectan rutas, técnicas o timing,
  salvo una corrección opcional explícita. No introducir cambios de balance ocultos.
- Corregir pérdidas de guardado, recursos agotados, desaparición de elementos
  esenciales y crashes del port. Reproducir antes y después.
- Una lectura NULL o fuera de rango en C no prueba un crash en GBA: el bus del
  hardware puede devolver un valor sin generar una excepción.
- Registrar ROM/región, estado inicial, entradas por frame, versión del emulador,
  resultado y comparación con hardware cuando exista. Un estado del port no es
  una prueba de que el cartucho tiene el mismo fallo.
- Un futuro modo de fidelidad debe reproducir resultados originales mediante
  código definido y comprobable; no reintroducir corrupción de memoria de Windows.
  Esta política está documentada; el modo opcional todavía no está implementado.

## Técnicas documentadas por jugadores/autores

| Caso | Evidencia | Reproducción propuesta | Decisión provisional |
|---|---|---|---|
| BBA de Riku en Dark Mode | super_bacon807 describe que el ataque B+B+A no descuenta usos de ciertas cartas de enemigo limitadas por ataques | Activar Dark Mode y una carta con usos por ataque, ejecutar B+B+A y comparar contador con un ataque normal | Preservar; pendiente de reproducción local por región |
| Dark Aura interrumpida y Sleight Lock | El mismo autor describe golpes obtenidos antes de una interrupción sin descontar el uso del efecto | Con Sleight Lock activo, provocar que el enemigo interrumpa Dark Aura antes del final; comparar contador | Preservar de momento; puede ser semántica del contador y no un error |
| Ataque antes del final visual de un card break | El autor del TAS USA documenta una ventana de dos frames | Registrar aceptación de A alrededor del final de la animación en USA y comparar port/emulador | Preservar timing; no clasificar automáticamente como bug |
| Cancelaciones favorecidas por el suelo ácido de Parasite Cage | El autor del TAS usa el daño del suelo para acortar combos | Comparar animación y aceptación de entradas con daño del suelo y sin él | Preservar interacción; no clasificar automáticamente como bug |

Fuentes primarias:

- [Guía Reverse/Rebirth de super_bacon807](https://gamefaqs.gamespot.com/gba/919011-kingdom-hearts-chain-of-memories/faqs/49605).
  Consultada mediante resultados indexados: la lectura directa de la página no estuvo disponible.
- [Comentarios de Doc_Skellington sobre su TAS USA](https://tasvideos.org/4773S).
  La prueba usa VBA-rr; no confirma por sí sola otras regiones ni hardware físico.

## Hallazgos del código que requieren contraste con el cartucho

Los comentarios `@bug` del árbol local son pistas de revisión, no certificados
de fallos visibles. Algunos fueron añadidos durante el trabajo del port.

| Grupo | Archivos | Qué investigar | Tratamiento |
|---|---|---|---|
| Nombres de combos Sora/Riku | `src/card/card_friend_card.c` | Lista de seis nombres sin centinela; en EU, recursos del nombre anterior mezclados con el índice nuevo | PC tiene corrección de límites/recursos; pendiente de comparación visible en GBA |
| Teclado alemán EU | `src/card/card_deckmenu2.c` | Copias fuera del bloque de tiles y cálculo de pestaña | Reproducir en alemán; corregir defectos molestos sólo tras verificar el resultado original |
| Gráficos alemanes de menús | `card_level_up.c`, `map.c`, `mode_worldwarp.c`, `mode_worldinspect.c` | Tamaños de copia mayores que los bloques de recursos | Comprobar si altera pixels en cartucho o sólo lee datos adyacentes inocuos |
| Textos largos de baraja Riku | `include/card/card.h` | Descripción mayor que la capacidad de 60 slots señalada en el código | Corregir límites de memoria en PC; preservar texto completo |
| Recursos de intercambio de barajas | `src/card/card_deckexchange.c` | Segundo release de una paleta; comprobar referencias y ejecución real | Evitar liberación indebida, sin alterar jugabilidad |
| Primera carta de enemigo, índice 450 | `card_deckmenu2.c`, `card_prize_card.c` | Comparación estricta en vez de inclusiva; parte de las rutas está marcada como no llamada | Reproducir sólo en rutas alcanzables; no afirmar que se pierde una carta sin evidencia |
| Límite de lista de mundos | `src/menu/mode_worldinspect.c` | Escritura posterior al último elemento al llenar pisos | Corregir memoria en PC; verificar selección y orden originales |
| Recarga y huecos vacíos | `card_battle.c`, `card_battle_riku.c` | Accesos a definiciones NULL que el bus GBA tolera | Emular resultado definido; un crash Windows no debe preservarse |
| Divisiones por cero | `map_cell.c`, `bos_pc.c`, `engine.c`, `card_worldselect.c` | Plataformas estrechas, escalas nulas y páginas vacías | Mantener resultado original de libgcc/bus mediante código seguro |
| Confirmación del menú de campo | `src/map/map.c` | Recursos de confirmación que pueden permanecer asignados al destruir el menú | Probar entradas/salidas repetidas; corregir fugas si se confirman |

Para inventario técnico completo: `rg -n '@bug' src include`. Revisar también
el `#ifdef` y si la ruta se ejecuta; los comentarios con `@bug?` son hipótesis.

## Errores que no deben confundirse con bugs originales

- **Cristal del castillo desaparecido:** reproducido en la build de referencia
  del port tras interacciones repetidas desde la ranura 8. Hay una corrección
  del ciclo de tareas de salas fijas en preparación; todavía falta regresión.
  No está confirmado como bug del cartucho.
- **Guante/texto estirados, cyan, headers incompletos, popeos:** regresiones de
  presentación panorámica del port, fuera del modo de preservación.
- **Crash al entrar en Añadir naipes:** corrección PC de contadores iniciales
  validada. No hay prueba de que la GBA tenga el mismo crash visible.
- **Renderizador HD de emulador:** [mGBA #1620](https://github.com/mgba-emu/mgba/issues/1620)
  documenta una diferencia gráfica con la ruta OpenGL HD. No usarla como prueba
  de un glitch del cartucho; comparar primero con su renderizador de software.
- **Intro congelada o guardado fallando en emuladores/flashcarts:** no se incluyen
  como bugs originales sin identificar dispositivo, versión y reproducción.

## Próximo cierre del catálogo

- [ ] Reproducir BBA y Dark Aura/Sleight Lock en las tres ROM suministradas.
- [ ] Contrastar los hallazgos del código con el decomp upstream y resultados reales.
- [ ] Añadir pruebas de conservación para cada técnica confirmada.
- [ ] Separar correcciones técnicas obligatorias de mejoras opcionales de gameplay.
