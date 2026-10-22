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

## Traspaso (22/10 ~06:30, al Claude anterior se le acaban los tokens): cómo seguir

Herramientas (en el repo): `khhd_textures.py` (saca un asset + sus texturas HD), `hdhud_gba_card_sheet.py`
(dibuja cada ilustración de carta de GBA desde la ROM: marco + tiles 4bpp crudos + paleta, símbolos de
`build/eu/pc/rom_symbols.s`), `hdhud_gba_shots.py` (capturas GBA de combate y del menú de barajas). Material de
trabajo (fuera del repo, no se sube: es del juego del jugador): `D:/01Claude/ReKHCoM-build/hdhud/` con `cards/` (las
226 cartas con sus DDS HD), `families.png` (una muestra por familia), `gba_cards.tsv` (119 cartas distintas de GBA:
gráfico, nombre, kind, categoría, número de catálogo), `gba_cards.png` (70 dibujadas), `recom.txt` (nombres de OpenKh).

Pasos siguientes, en orden:
1. 49 cartas de GBA no salen en la hoja porque sus símbolos `...Frame0/Tiles/Palette` no están en `rom_symbols.s`
   (amigos/invocaciones Smn*, enemigos Emy*, jefes Bos*, Npc*): buscar su nombre real (assets del decomp, `asm/`,
   `gen/*.h`) o si los tiles están comprimidos.
2. Mapear cada carta de GBA a su archivo de Re:CoM comparando la hoja GBA con `families.png`/`cards/` (por imagen,
   no por número: GBA llama `gCardWep04` a Three Wishes y Re:CoM `cw_002`). Guardar la tabla en
   `port/data/hd_cards.tsv` (id de GBA → archivo de Re:CoM, rectángulo del marco en `com_card01`).
3. Geometría: medir en GBA el marco (32x? con corona), la ventana de la ilustración y el círculo del valor; recortar
   el marco de `com_card01` por color de categoría y los dígitos de `card_no`.
4. Importador: `khhd_cards.py` + el mismo paso en el launcher (`port/decompui/hd_import.inc`, junto a la música y
   los videos): DDS (DXT, decodificar con un decodificador BC chico, p. ej. bcdec) → PNG sin pérdida, recortado, solo
   las cartas usadas, en `hd/cards/`. Comprimir al máximo sin pérdida (zlib 9; paleta si entra en 256 colores).
5. Juego: opción `hd_cards` (juego + launcher + ini, Mejoras de Re:CoM, beta, apagada por defecto salvo Modo HD);
   etiquetar los 3 sprites de cada carta (`EnemyCardDraw` y parientes de Sora/Riku/menú de barajas: buscar
   `gCardValueDigitFrames` y `cardDef->gfx`), capas en `PortPpuRenderHud` y quads en la presentación (diseño arriba).
6. Pruebas: opción apagada = capturas idénticas (`compare_present_builds.py`); encendida: combate EU/US/JP, mano con
   cartas superpuestas, carta activa girando, menú de barajas; sin cartas faltantes ni orden roto.
7. Proponer al usuario los tentativos (medidores, contador, recarga, fuentes HD de Re:CoM) con capturas.

## Estado (22/10 ~06:30)
- [x] Ubicación y formato de las texturas HD; familias identificadas; qué es 1:1.
- [x] Mapeo de las 97 ilustraciones de carta de GBA (96 de `gCardDefs` + `gGimmickCard`) a Re:CoM, a ojo (el
  emparejamiento automático por parecido falló: los dibujos de GBA no son reducciones de los de Re:CoM). Marcos
  rojo/verde/azul/negro (`com_card01`) sobre la caja de tinta del borde de GBA; dígitos blancos de `card_no` (los
  modificados teñidos del azul de GBA). Receta: `port/data/hd_sprites.tsv`; herramientas `hdcards_gba.py` (dibuja
  los sprites de GBA desde la ROM) y `hdcards_parts.py` (marcos y dígitos).
- [x] Extracción en Python: `khhd_sprites.py` (receta → `hd/sprites/<nombre>.png`, 121 piezas, 8 MB). Formatos:
  `khhd_scan.py` (todas las texturas HD del .pkg), `khhd_imgz.py` (IMGZ/IMGD de 32 bits).
- [x] Mismo paso en el importador del launcher (`hd_import.inc`, PNG con miniz; `ReKHCoM.exe --hd-import <juego>
  --sprites-only` para pruebas): idéntico a Python (diferencia máxima 1 por redondeo).
- [x] Juego: `hd_cards` (ini); `engine.c` da a cada parte de OAM sus tiles/marco/caja (`PortOamSetHdPart`),
  `port_hdsprites.c` las reconoce por los tiles de la ROM y las dibuja como quads con la transformación del juego;
  lo que el cartucho dibuja delante va a una capa encima (`PortPpuHudOver`). EU/US/JP, mano, cartas enemigas
  girando, jefe: sin caídas (el renderer por software de SDL se colgaba con quads espejados: ver DEBUG_WORKFLOW).
- [ ] Opción en Opciones/launcher (Mejoras de Re:CoM, beta), modos GBA/HD.
- [x] Menú de barajas y vista Original (22/10): las piezas HD de la imagen principal quedan detrás de lo que el
  cartucho dibuja delante (`MainHdOver` en port_ppu.c: esos píxeles van a una capa encima, `PortPpuMainOver`).
  Las cartas chicas del menú no tienen ilustración 1:1; solo el dígito es HD.
- [x] Pruebas: con las opciones HD apagadas, 12/12 capturas idénticas a antes del HD (`compare_present_builds.py`, 22/10).
- Regla del usuario (22/10): en las cartas, los dígitos HD originales de Re:CoM (`card_no`). La fuente del texto HD es
  solo para HUD y diálogos; probada en las cartas y rechazada ("feísimo").

## Título HD (22/10, `hd_title`, beta)
El logo del título de GBA son sprites (`task_title_logo_2`): corazón, letras KINGDOM HEARTS, corona, Disney/Square
Enix y la placa CHAIN OF MEMORIES. Re:CoM los tiene 1:1 en `FORM/0505/FO0505.RTM:UK_title.imz` (IMGD 3): el corazón
plateado, KING + DOM HEARTS (compuestas con `canvas`/`blit` donde caen sobre las letras de GBA: escala 1,01/0,99 a 4x),
la corona y Disney/Square Enix; la placa es la cinta "Re:Chain of Memories" sin el "Re:" (pedido del usuario: cortar y
mover). En la PPU son sprites de la imagen del juego (capa 0): `QueueMainHdParts` los encola al empezar el cuadro y
`DrawObjs` los deja fuera; se dibujan sobre la imagen terminada (se supone que van delante de todo).
Pendiente: la intro con zoom usa el logo como fondo afín (`gTitleLogoBg`), sigue en GBA; "DESARROLLADO POR...", "PULSA
START" y el arte de Sora no tienen equivalente 1:1.

## Cartas de mapa (22/10, estudiado, para después de la v2.0)
Las 22 ilustraciones de Re:CoM (`CARD/06xx/CA06xx.RTM:cr_###_tm2.dds`) son las mismas que las de GBA (`gCardRoom##`,
caja -16 -12 32 32, arte en 3,2..29,30 como las de combate: sirve la receta `crop=9,0,247,256;place=3,2,29,30;scale=8`).
Key to Rewards (`gCardRoom23`) no tiene. GBA→cr: 01→002 02→001 03→018 04→006 05→005 06→020 07→003 08→004 09→017
10→019 11→022 12→011 13→010 14→012 15→013 16→021 17→015 18→016 19→014 20→007 21→008 22→009 (carpetas: cr_001..009 =
0601..0609, 010..016 = 0621..0627, 017..022 = 0641..0646). Mundos: `cl_001..013` (CARD/0301..).
**Hecho 22/10:** las 22 cartas de mapa y sus marcos (los tonos oscuros y el dorado de `com_card01`, que Re:CoM usa
para el mapa) en HD, también en la síntesis de estancias, que va en la capa de pantalla: las piezas HD de la capa de
pantalla y su oclusión (`HdOverLayer`, `PortPpuScreenOver`). Antes: la síntesis de estancias las dibuja en la capa principal, con el guante del cursor y el círculo del
valor encima; la capa principal HD se pinta sobre todo. Hace falta oclusión en la capa principal (repintar encima los
píxeles de lo que va delante, como `sHdCover`/`sHudOver` del HUD) antes de añadirlas como grupo propio.
Prueba: `KHCOM_MODE='60:room:10901,120:fieldfreeze,200:doorpose:0' KHCOM_KEYS='240-242:A'` (test_door_views.py).

## Mundos (22/10, hecho)
Los 13 `gWorldImage*` de la selección de mundo son los `cl_###` de Re:CoM (CARD/03xx), sobre un brillo azul opaco.
Grupo 4 de la receta: el juego recorta la imagen HD a la silueta del sprite del cartucho al cargarla
(`MaskBySprite`, borde suave de un píxel de GBA). Recetas por ajuste de cajas (crop = caja del objeto HD, place = caja
del sprite de GBA). Prueba: `KHCOM_MODE=200:worlds:8191,300:mode_worldselect` (13 mundos a la vez llenan la memoria de
sprites y faltan letras del texto: artefacto de la prueba, también sin HD).
