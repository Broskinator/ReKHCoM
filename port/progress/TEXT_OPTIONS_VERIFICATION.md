# Texto HD y opciones — evidencia del 14/10/2026

## Opciones

`port/tools/test_text_options.py` ejecuta rutas en el menú pausado mediante
`Change()`, la misma función del teclado/mando. Cada ejecución usa ejecutable,
configuración y bancos propios; no copia ni escribe las partidas del usuario.
El harness es optativo (`KHCOM_OPTIONS_ACTIONS`), sin efectos cuando falta.

- EU EN/FR/DE/IT/ES, US EN y JP JA: 96 cambios por ejecución, 23 filas
  seleccionables (excluye cabecera y Volver), 672 cambios y 672 BMP. Salida 0,
  sin CRASH, regreso al juego capturado. Evidencia inicial:
  `build/codex-text-options-results/20261014_035143_269405`.
- Cubre vistas, zoom, escalado, tamaños HUD, escenas GBA, tres filtros,
  fullscreen/resolución, VSync, scanlines, LCD, altura cartas, FPS, acciones
  de una tecla, bancos, volúmenes, HD text, HD-2D y audio suave. Ejercita zoom
  deshabilitado en vista original y escalado deshabilitado en Auto.
- Revisión visual individual: menú alemán y japonés, incluyendo scroll hasta
  Experimentales; cifras y etiquetas no cortadas en las capturas revisadas.
- Prueba adicional **Windows real**, EU EN, 96 cambios/96 BMP, valida archivo
  INI guardado y consulta el estado efectivo de SDL. Fullscreen 1920×1080,
  cambio exclusivo 1600×1024, regreso a ventana 640×360; VSync 0 y 1.
  `build/codex-text-options-results/20261014_040107_060996/result.json`
  (el informe está dentro de `eu-en`).

Las primeras siete ejecuciones usan vídeo dummy y solo demuestran cambios del
menú/renderizado, no el cambio físico de monitor. La prueba Windows prueba ese
camino real en EU; falta volver a ejecutar las otras seis combinaciones con
el harness final y comprobar efectos en escenas (cartas/HUD, sonido y acciones).
No es evidencia suficiente por sí sola para declarar la barra al 100%.

## Inventario de texto

| Familia | Ruta | Tratamiento |
|---|---|---|
| Diálogos/tutoriales y nombres/descripciones cartas | `src/msg/msg.c`, glyph codes y card glyph codes | Sustitución opcional existente |
| TextSlots, estadísticas y texto de menú | `src/msg/msg.c`, `PortSlotRemember` y `DrawTextSlots*` | Sustitución opcional existente |
| Subtítulos | msg/movie glyph codes y `PortOamSetText` | Sustitución opcional existente; estilo 1 |
| EXP/nivel/experiencia siguiente (cinco dígitos) | `src/btl/btl_exp.c`, `BtlExpSetNumber` | Nueva etiqueta estilo 2; tipografía pequeña en celdas de 8 px |
| Cuenta/CP y cantidades colección en barajas Sora/Riku/intercambio | `card_deckmenu2.c`, `card_deckmenu2_riku.c`, `card_deckexchange.c` | 48 transferencias BG etiquetadas; backend PPU y verificación en curso |
| Dígitos valor/premium/modificado de una carta | `gCardValueDigitFrames`, `gCardPremiumValueDigitFrames`, `gCardModifiedValueDigitFrames` | Parte del dibujo de carta; conservar colores, animación y afinidad |
| Contador de recarga | `gReloadCounterWhiteAnims` | Indicador animado del juego; conservar su semántica visual |
| Logos LEVEL UP, EXP/NEXT LEVEL, GUARD/MISS/CARD BREAK | Sprites compuestos/animados propios | Arte diseñado, conserva original |
| Iconos flechas/botones y debug | Códigos de control msg y `DebugTextPrint*` | Iconos originales; debug queda fuera del texto de juego |
| Opciones nativas | `port_ui.c` | Fuente pixel del juego, traducciones completas y fuente japonesa de cartucho |

No se confunde el número dibujado en un naipe con una cifra estadística de
texto. Modo pixel sigue disponible; ningún cambio modifica el cartucho GBA.

## EXP: implementación y pruebas

Nuevos tags solo identifican los cinco dígitos reales de los assets regionales;
el sexto sprite de sufijo y el rótulo permanecen originales. Se restaura
`gPortTextChar/gPortTextStyle` tras cada DrawSprite. Estilo 2 usa capital de
5 px, baseline 7, celdas fijas de 8 px; no aplica flujo de palabras.

`test_text_options_hd.py`: seis runs EU/US/JP × HD off/on, valores 12345 y 67890
mediante task de batalla real, 12 BMP, salida 0 sin CRASH, todas las cifras
cambian al activar HD. `build/codex-text-options-hd/20261014_035944_148776`.
La revisión encontró colores/fondo incorrectos: los números no usan la paleta
1/2 del msg sino 15 blanco/14 sombra/13 panel azul. El root corrigió esa
convención del PPU; la evidencia anterior es diagnóstico, **no aprobación
visual**. Repetición y revisión del resultado corregido pendientes.

La oclusión general se coordina con el root (PPU). Los fixtures de cifras no
demuestran todavía tutoriales ni toda la partida.
