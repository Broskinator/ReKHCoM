# Cola vigente (21/10/2026, reparto con Codex)

Publicado: v1.9.12 estable (Latest) con Windows, Linux, Steam Deck, Anbernic, Android, PS5, PS4 y ahora las alphas de Switch y Vita; web en Pages. Issues #23, #24, #30 y #31 cerrados; #25 abierto. PR #32 integrado.

Prerelease de prueba v1.9.13-pre2 (21/10, pedido del usuario): Windows + APK (32 y 64 bits) + Switch .nro (GPU).
Comentado en #22, #33, #34 y #35 pidiendo que prueben y dejen modelo/versión de su equipo; se cierran con su
confirmación. Copia local: `Probar ReKHCoM v1.9.13 pre2.bat`.

Única cola de trabajo; los demás .md son evidencia. Lo terminado sale de aquí
(queda en el historial de git y en CHANGELOG.md). Numeración histórica conservada.

Reglas del usuario:
- Claude y Codex commitean y pushean a main; si uno lleva más de 10 min sin commits ni
  archivos tocados, el otro toma sus puntos anotando hora y evidencia aquí.
- Todo lo visual se entrega como copia jugable + .bat y espera aprobación del usuario; lo
  aprobado va a LOGICA_APROBADOS.md; método en PRUEBAS_VISUALES.md.
- La validación en hardware real no es pendiente: llega como reporte.
- Nunca borrar carpetas ReKHCoM-vX sin que el usuario lo pida.

## Traspaso 21/10 11:10

El usuario cambió de chat: un Claude nuevo toma los puntos del Claude anterior (sin cambios desde
las 11:04) y sigue de integrador de Codex. El WIP de música HD en los hosts con puente quedó commiteado
tal cual (build PC verificada); falta lo anotado en `HD_MOD.md` (emulador Android 64, Switch/Vita/PS5 en la VM).

## Traspaso 21/10 12:55

Otro Claude nuevo retoma (el anterior sin actividad desde las 12:53). La batería Android 64 se cortó a las 12:53
porque adb no arrancó (0xC0000142, no es fallo del juego); se retoma desde el combate 30 de EU. Codex integrado hasta
457d10854 (Atlántica 102).

## Traspaso 21/10 ~15:15 (Claude nuevo retoma I y J en `claude/perf-android-reloc`)

- J: el `shareLog` de `GameActivity.java` no compilaba (saltos de línea reales dentro de las comillas); arreglado. APK
  de prueba EU con la imagen movida (`KHCOM_IMAGE_MOVE=1`): carga en 0xe2600000 y el menú de estado sale bien; la
  batería menús/fixtures/jefes con la imagen movida corre en `build/android-scenarios/report-eu-khcom_image_move.json`
  (`test_android_scenarios.py --env`).
- #37 (no se podía salir de "Press..." sin teclado): en la captura de teclado, un botón del mando, un toque o Atrás la
  cancelan. #38: con "ocultar controles táctiles" y un mando conectado no se dibuja nada, ni en menús (sin mando queda
  el botón de opciones, la única entrada al menú). Solo Android; Switch y web igual que antes.
- I: verificación de la PPU en varios núcleos con `KHCOM_PPU_VERIFY=1` sobre los eventos US en
  `build/ppuverify/us` (`test_events.py --work`, aparte de la batería de escritorio, que usa `build/pc-eu`).

## Traspaso 21/10 ~15:10 (al Claude o Codex que siga; tokens del usuario casi agotados)

- Rama `claude/perf-android-reloc` (sin verificar, NO está en main): PPU en varios núcleos (I) y arreglo de Android 64
  (J). Antes de llevarla a main: (1) `KHCOM_PPU_VERIFY=1` en la batería de eventos/salas/combates con 0 `MISMATCH`;
  (2) emulador Android con y sin `KHCOM_IMAGE_MOVE=1` (menús y jefes); (3) build pc-eu y prueba de combate.
- RAM: con las baterías y los programas del usuario quedan ~0,7 GB; el emulador necesita ~3 GB: pausar las baterías
  (se retoman solas con `run_batteries.py --detach`) y pedir al usuario que libere RAM.
- #36 (Switch): comentario listo en `build/issue36-comment.md`, se publica junto con la próxima prueba.
- Vita a la GPU: hace falta permiso del usuario para `vdpm sdl2` en la VM (`RENDIMIENTO.md`).

## Baterías 21/10 13:13

Las baterías se cortaron dos veces al cerrarse la sesión de Claude (Windows mata el árbol de procesos; no son
fallos del juego). Desde 2f37b099f corren con `python port/tools/run_batteries.py --detach`, fuera de la sesión
y reanudables caso por caso, en pasos chicos (batería × versión); `--status` muestra el avance. Ver `BATERIAS.md`. Orden (pedido del usuario: eventos y mundos antes de
pedir nada a los testers y de dar una beta): kit de escritorio events/wide/rooms EU/US/JP, luego Android 64 (EU jefes
desde el 30, Riku, US/JP menús y jefes).

## Reparto

| Quién | Pendiente |
|---|---|
| **Claude** | **A. Batería Android 64** (corriendo 21/10 en el emulador Android 14: EU menús/fixtures/jefes, EU jefes con Riku, US/JP menús y jefes). Al terminar: rehacer el APK de prueba y repetir las 7 pantallas agregadas al arnés (f15eee5dd y anterior); arreglar cada fallo real; luego otros perfiles (Android 13/15, tablet, plegable) y el bot de partida. Estado en `ANDROID_64.md` |
| **Claude** | **B. Batería de escritorio** con el kit (`debug_kit.py run --build --versions eu,us,jp`, `--jobs 2`): Riku, eventos, eventos 16:9, juego aleatorio; cada `bios:` nuevo se arregla (#30, `KIT_DEPURACION.md`) |
| **Claude** | **C. Videos HD en Android**: rama `claude/android-hd-movies` (rebasada sobre main 21/10); el emulador se cuelga en la primera textura YUV (`glCreateProgram`). `KHCOM_HDMOVIE_RGB=1` (b4cb4730d) convierte a RGB en la CPU: probarlo en el emulador cuando acabe la batería; a main solo verificado (`HD_MOD.md`) |
| **Claude** | **D. Videos HD en consolas y Linux** (Switch/Vita/PS4/PS5, MP4 en Linux): después de Android |
| **Claude** | **E. Issues de la pre2** (#22, #33, #34, #35; #36 nuevo 21/10: Switch V1 se cierra al cargar el título, sin versión ni log: comentario con preguntas listo en `build/issue36-comment.md`, no se publica todavía: el usuario quiere terminar antes la batería de eventos y mundos y dar una beta): leer las respuestas de los testers, arreglar lo que reporten y cerrar con su confirmación; #35 Switch: si sigue lento con la GPU actual, evaluar deko3d. #25 (escenas HD): agradecer a DL-6-case y cerrar cuando salga el mod HD en una estable |
| **Claude** | **F. Discord Rich Presence**: hecho, con el Application ID del usuario (718b5d212, 20/10); falta probar Linux |
| **Claude** | **G. Integración de Codex**: traer sus commits de `khcom-codex` (rama codex/preliminar-v1) y `khcom` a main con build y prueba antes de cada push (al día hasta 457d10854, 21/10) |
| **Claude** | **I. Rendimiento** (pedido del usuario 21/10, `RENDIMIENTO.md`): PPU en varios núcleos (x1 en Windows a 60 fps con margen; toma la parte de rendimiento de la PPU, zona de Codex), Vita y PS4 a la GPU |
| **Claude** | **J. Android 64 en teléfonos reales** (rama `claude/perf-android-reloc`, compila; falta emulador: `KHCOM_IMAGE_MOVE=1` fuerza la reubicación; luego APK de prueba y pedir a #22/#34 que prueben; el error ahora tiene botón "Share log" con modelo, Android, páginas y mapa de memoria) (#22 Poco F8 Ultra, #34 Mate XT: "The game image could not be loaded"): la imagen se reubica donde haya hueco si 0x01000000 está ocupada (`ANDROID_64.md`); probar en el emulador y dar otra prueba a los testers |
| **Claude** | **H. Próxima estable**: con el OK del usuario; `release_check.py` antes/después, todas las plataformas (Switch y Vita incluidas), web, CHANGELOG ya al día con v1.9.13 |
| **ChatGPT/Codex** | 39: rutas completas de Sora y de Reverse/Rebirth |
| **ChatGPT/Codex** | 16–24: arte de jefes y escenas, de a uno con antes/después, con el método de abajo ("Para Codex: cómo hacer el arte") |
| **Usuario** | Probar `Probar Discord.bat` y `Probar ReKHCoM v1.9.13 pre2.bat` (pantalla completa sin parpadeo, mod HD, volver al título) |
| **Usuario** | Aprobar el launcher nuevo (`Probar launcher nuevo.bat`, commit 07a6fa9ab); revisar 61–62 cuando Claude las muestre |

## Para Codex: cómo hacer el arte (16–24), según lo aprobado (21/10/2026)

Pedido del usuario: seguir el mismo método con que se aprobaron los arreglos visuales.

- **Procedimiento:** `WORKFLOW_ARTE_AMPLIO.md` (paso a paso), reglas en `LOGICA_APROBADOS.md`
  (mandan ante la duda), entrega en `PRUEBAS_VISUALES.md`, técnica de combates en
  `BATTLE_SCENERY_TECHNIQUE.md`.
- **Modelo "aprobado con honores" (20/10): Guard Armor, escena 008.** El patio del combate cubre
  también el evento previo: la extensión se elige por los cuatro mapas del fondo, no por el modo
  (`port_ppu.c`, `guardPatio`). Lección: buscar TODAS las apariciones de un fondo (combate, evento
  previo, derrota, revancha) e identificarlo por sus datos, nunca por modo/número de combate/bandera.
- **Arte:** solo tiles del juego (repetir/voltear); bordes cortados se cierran con el borde propio
  volteado; nunca ornamentos repetidos o cortados a la mitad (relleno de pared lisa); pixel art
  nuevo solo donde falte, con píxeles y paleta del juego. El rectángulo GBA queda idéntico.
- **Pruebas:** zoom x3 entero (`zoom=6`) a 1920x1080, 2560x1080, 3440x1440 y 3840x1080
  (`KHCOM_WINDOW`), antes/después.
- **Entrega:** copia jugable + `.bat`, de a un jefe/escena, y esperar la aprobación del usuario;
  lo aprobado se documenta en `LOGICA_APROBADOS.md`.

## Vita y Switch (20/10, 17:00)

Por qué es distinto de PC/PS5/PS4: el juego lee los datos del volcado del usuario, y esos datos
llevan punteros absolutos de GBA (0x08xxxxxx). PC, Android, web, PS5 y PS4 cargan la ROM en
0x08000000 y los punteros valen tal cual. Switch (AArch64, código desde 0x08000000, homebrew sin
procesos de 32 bits) y Vita (ARM32, usuario desde 0x81000000) no pueden usar esas direcciones.

Nuestro método (PORT_GBA_RELOC, `include/gba/gba_space.h`): todas las regiones GBA son arreglos del
programa; al cargar la ROM se corrigen sus punteros con la lista exacta del enlazador
(`port/data/rom_pointers_*.inc`, de reenlazar la ROM del decomp con `--emit-relocs`), y los
accesos sueltos a direcciones de bus se traducen (`GbaSpacePtr`). Una misma imagen sirve a las dos.
ggdrn/KH_COM_VITA (MIT) hace lo equivalente de otra forma: enlaza los datos dentro del ejecutable
(el enlazador de Vita corrige los punteros), los pone a cero y los rellena desde la ROM del usuario
al arrancar; los punteros dentro de bloques binarios los traduce con `GBA_PTR()` en cada uso.
Las dos vías necesitan que el decomp compile la ROM idéntica al cartucho.

Hecho:
- Modo reubicable: cabeceras, símbolos fijos (`rom_symbols_reloc.s`), build de comprobación en
  Windows (`configure_pc.py --reloc`) con el rango del bus reservado sin acceso.
- Build idéntica de GBA recuperada (7c7fb0f09, 62777ed3f): ROM EU/US/JP iguales al cartucho.
- Listas de punteros de EU/US/JP (`gen_rom_pointers.py`, 132037/98603/98035 palabras), cada una
  comprobada contra el cartucho; incluyen las desalineadas (GOTO/PATT de las canciones).
- Switch (`port/switch`, README): imagen AArch64 de punteros de 32 bits para Cortex-A57 (sin
  `-mcpu` clang emitía `xpaci` de ARMv8.3 y caía), memcpy/memset de 16 bytes, `.nro` de las tres
  regiones (`build_switch.py host`, devkitA64 de la imagen Docker devkitpro/devkita64 porque
  apt.devkitpro.org responde 403).
- Vita (`port/vita`, README): misma imagen reubicable en ARMv7 (clang, Cortex-A9), aislada en un
  objeto con solo `guest_main`/`gHostTable` globales y enlazada con el host compartido
  (`switch_host.c` con `__vita__`: rasterizador a 960x544, sceAudioOut, sceCtrl); un ejecutable por
  región (eboot.bin EU, eboot_us/jp.self). VitaSDK en la VM de consolas. `build_vita.py test` corre la
  misma imagen en Linux ARM de 32 bits.
- Verificado en la VM ARM64: título, menús y la batalla KHCOM_BATTLE=10:4 en Switch y en Vita; el
  fotograma 1490 sale idéntico píxel a píxel en las dos imágenes.

Sin verificar (no hay consola ni emulador aquí): mapeo de memoria de código en Switch real, pantalla,
audio y mandos del host de Vita, rendimiento. Llega como reporte de quien lo pruebe.

Siguiente:
1. Switch: PUBLICADA como alpha dentro de la release v1.9.12 (`ReKHCoM-v1.9.12-switch-alpha.nro`,
   pedido del usuario 20/10; archivos viejos intactos). La próxima release la lleva desde stage_release.
   Vita: PUBLICADA igual (`ReKHCoM-v1.9.12-vita-alpha.vpk`, 20/10).

## Esperando a terceros (no se trabaja)

- #23 mandos Nintendo: opción y normalización listas en juego y launcher; falta reporte con mando real (Switch Pro, 8BitDo, SN30).
- Issue #27 (scanlines en márgenes): abierto hasta que quien lo reportó confirme.
- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
