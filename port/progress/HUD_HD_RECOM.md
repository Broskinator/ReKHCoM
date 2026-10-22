# HUD HD y cartas HD desde el Re:CoM del 1.5+2.5 (22/10/2026)

Pedido del usuario: usar los gráficos HD de Re:Chain of Memories (KINGDOM HEARTS -HD 1.5+2.5 ReMIX- de PC del
jugador) donde sean **1:1** con los de GBA (o se ajusten 1:1), con una opción nueva **Cartas HD** aparte del HUD HD.
Los tentativos se le proponen al usuario aparte. Todo se importa con el mismo importador del mod HD (Steam/Epic o
carpeta elegida); nada del juego viaja con ReKHCoM. Guardado sin pérdida y liviano (PNG optimizado, solo lo que se
usa, recortado). La v2.0 espera a esto como beta (y al arte de Darkside).

## Dónde está (Recom.hed/.pkg, formato de OpenKh)

Nombres de archivo: `OpenKh.Egs/resources/recom.txt` de OpenKh (12690 nombres). Cada asset del `.pkg` trae su
textura de PS2 y **entradas remasterizadas**: las texturas HD que agregó el 1.5+2.5, en DDS (RGBA) o PNG.
`khhd_pkg.read_asset(..., with_remastered=True)` las devuelve.

| Archivo | Remasterizado | Qué es |
|---|---|---|
| `CARD/<n>/CA<n>.RTM` (226) | `cw_###` (19), `cm_###` (7), `ci_###` (8), `cs_###` (7), `cf_###` (8), `ce_###` (80), `cl_###` (13), `ck_###` (3), `cr_###` (22), `cg_001`, `cn###` (41): 256x256 | ilustración de cada carta: llaves espada (cw), magia (cm), objetos (ci), invocaciones (cs), amigos (cf), enemigos (ce), mundos (cl), llaves (ck), mapa (cr), Mickey (cg), personajes (cn) |
| `CARD/1000/CA1000.RTM` | `com_card01` 1024x1024, `com_card02` 512x512 | marcos de carta por color (rojo, verde, azul, gris, dorado) con corona, ventana de la ilustración y círculo del valor |
| (en CARD) | `card_no` 1024x128 | dígitos 0-9 del valor de la carta (dos estilos) |
| | `fo_###`/`fu_###` 256x512 | marcos de carta grandes (frente/dorso) |
| | `reload_no01-03`, `no_count`, `card_rev`, `pl_gauge`, `ene_gauge` | recarga 1-2-3, contador "CARD", dorso, medidores (tentativos: forma distinta a GBA) |
| `SYS/0001/SY0001.VTM` | `sysfont_fo2/fo20`, `evtfont_*`, `<idioma>_sysfont_fo240.png` | fuentes HD de Re:CoM por idioma (para el texto HD, tentativo) |

## Qué es 1:1 con GBA (comparado con capturas de combate y del menú de barajas, EU)

- **Cartas de combate** (mano de Sora/Riku, cartas del enemigo, carta activa): la carta de GBA es la de Re:CoM:
  marco de color con corona y círculo del valor abajo a la derecha, fondo azul, la misma ilustración (Llave del
  Reino en diagonal, el Sombra en la verde). → Cartas HD.
- **Cartas chicas del menú de barajas**: mismo diseño, más chico. → Cartas HD.
- Tentativos (para proponer): medidores de vida del jugador y del enemigo (otra forma), contador de cartas,
  dorso de la pila, recarga, fuentes HD.

## Cómo se dibuja una carta en GBA

Tres sprites con la misma posición, matriz afín (giro/escala) y prioridad (`EnemyCardDraw` en
`src/card/card_enemy.c` y sus parientes de Sora/Riku/menús): el marco por categoría (`gEnemyCardBacks` +
`tiles[category]`), la ilustración (`cardDef->gfx`) y el dígito del valor (`gCardValueDigitFrames[value]`).

## Diseño de Cartas HD (`hd_cards`, apagado por defecto, beta)

1. Importador: extraer las ilustraciones y los marcos/dígitos, recortarlos y guardarlos como PNG sin pérdida en
   `hd/cards/` (solo las cartas que existen en GBA, mapeadas por categoría e índice; tabla verificada a ojo con una
   hoja GBA vs HD).
2. Juego: cada `DrawSprite` de carta etiqueta sus sprites con la carta (categoría, id, valor, modificado).
3. PPU (`PortPpuRenderHud`, de atrás hacia adelante): los píxeles donde una carta queda arriba no van a la capa
   base; los de sprites que tapan una carta van a una capa encima.
4. Presentación: capa HUD base → cada carta HD como un quad de GPU (`SDL_RenderGeometry`) con la matriz afín del
   juego, de atrás hacia adelante → capa de lo que tapa. Sin trabajo por píxel en la CPU; vale para todas las
   resoluciones y zooms, PC y Android.
5. Si falta un archivo o se apaga la opción: la carta de GBA, sin cambios.

## Estado
- [x] Ubicación y formato de las texturas HD; familias identificadas; qué es 1:1.
- [ ] Mapeo de las cartas de GBA a los archivos de Re:CoM (hoja de verificación).
- [ ] Extracción en `khhd_*.py` y en el importador del launcher (DDS → PNG sin pérdida).
- [ ] Etiquetado de sprites de carta, capas y quads HD; opción en juego/launcher/ini.
- [ ] Pruebas: capturas con la opción apagada idénticas; encendida, sin cartas faltantes ni orden roto.
