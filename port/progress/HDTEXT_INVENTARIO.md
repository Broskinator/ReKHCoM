# Texto HD: inventario de texto y números

Inventario del 14 de octubre de 2026 para la barra «HD text». El texto HD
(`port/src/port_hdtext.c`) sustituye los glifos que el juego dibuja como
sprites marcados con su carácter (`gPortTextChar` en `src/msg/msg.c`): diálogo,
menús, nombres de cartas, ventanas de sistema y subtítulos de los vídeos.

## Ya en HD

| Origen | Dónde | Cómo |
|---|---|---|
| Diálogo y mensajes | `msg.c` (`sMsgGlyphCodes`) | Glifo por glifo, KHMenu / Yu Gothic UI (JP) |
| Ranuras de texto de menús | `msg.c` (`PortSlotCode`) | Cada ranura de texto con su código |
| Nombres de cartas | `msg.c` (`sCardMsgGlyphCodes`) | Glifo por glifo |
| Subtítulos de los vídeos | `mode_movie.c` | Comic Hearts, centrados como en el cartucho |
| Menú de opciones del port | `port_ui.c` | Fuente del juego al tamaño del juego (estilo del menú de depuración) |

## Números que siguen en pixel art

| Función | Archivo | Dibujo | Clase | Decisión |
|---|---|---|---|---|
| `DrawDeckCardCount`, `DrawRikuDeckCardCount` | `card_deckmenu2*.c` | Tiles de BG | Texto de menú | Pixel art por ahora: necesita detectar texto en BG (PPU, zona de Codex) |
| `DrawDeckCategoryCount`, `DrawCollectionCategoryCount` | `card_deckmenu2.c` | Tiles de BG | Texto de menú | Igual |
| `DrawValueCount`, `DrawSelectedValueCpCost` | `card_deckmenu2.c` | Tiles de BG | Texto de menú | Igual |
| `DrawDeckExchangeValueCpCost` | `card_deckexchange.c` | Tiles de BG | Texto de menú | Igual |
| `DrawKeyboardDeckNumber` | `card_deckmenu2.c` | Tiles de BG | Texto de menú | Igual |
| `DrawLevelUpStatDigits` | `card_level_up.c` | Sprites | Texto de menú | Candidato a `gPortTextChar`; archivo de Codex (subida de nivel en curso) |
| Valor de cada naipe en combate (`UpdateSoraCardValue`, `UpdateRikuCardValue`) | `card_battle*.c` | Sprites | Arte de la carta | Mantener: forma parte de la ilustración del naipe |
| Contadores de recarga (`Set*ReloadCounterAnim`) | `card_battle*.c` | Sprites animados | Arte | Mantener |
| Valor de cartas de jefe (`SetBossCardValue`) | `card_enemy.c` | Sprites | Arte | Mantener |
| Cartas restantes en combate (`UpdateRevCount*`) | `card_reload_children.c` | Sprites | Indicador estilizado | Mantener |
| `PrintNumber` | `card_name.c` | Texto de depuración | Depuración | No aplica |

Criterio (PENDIENTES.md): no sustituir dibujos completos por glifos. Los
números de las cartas y los contadores animados son ilustración. Los números de
los menús de baraja son texto, pero el cartucho los escribe en tiles de fondo;
reemplazarlos pide reconocer esos tiles en el compositor y se coordina con Codex
cuando cierre los menús.

## Regiones

- EU: KHMenu para los cinco idiomas (acentos y ¿¡ nativos).
- US: KHMenu.
- JP: Yu Gothic UI sobre la rejilla del juego (Shift-JIS verificado en la intro).
