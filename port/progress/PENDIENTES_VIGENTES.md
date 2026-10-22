# Cola vigente (21/10/2026, reparto con Codex)

Única cola de trabajo; los demás .md son evidencia. Lo terminado sale de aquí (queda en git y en CHANGELOG.md).

Reglas del usuario:
- Claude y Codex commitean y pushean a main; si uno lleva más de 10 min sin commits ni archivos tocados, el otro toma
  sus puntos anotando hora y evidencia aquí.
- Todo lo visual se entrega como copia jugable + .bat y espera aprobación del usuario; lo aprobado va a
  LOGICA_APROBADOS.md; método en PRUEBAS_VISUALES.md.
- La validación en hardware real no es pendiente: llega como reporte.
- Nunca borrar carpetas ReKHCoM-vX sin que el usuario lo pida.
- Publicar (releases, comentarios en issues) solo con el OK del usuario.

## Estado (21/10 ~19:15)

- **v1.9.14 publicada (estable, Latest)**, commit a97268171 (main): PC (Windows, Linux, Steam Deck) estable; Android,
  Switch, Vita, PS4, PS5 (PPSA98511.zip, etiqueta ps5-beta) y Anbernic como `-beta`. Incluye J, #37, #38, I (PPU en
  varios núcleos, 90 eventos sin MISMATCH), mod HD (beta). Android 64 con imagen movida: 37/37 ok. Comentado en #22,
  #25, #33-#38; #25 cerrado. Los demás se cierran cuando el reporter confirme. Releases v1.9.12 y v1.9.13-pre2: borrar
  (el clasificador bloqueó `gh release delete`; lo hace el usuario). Escenas Preliminar V1 de Codex sin aprobar: NO
  integradas (rama claude/integrate-20261021).


- **Publicado:** v1.9.12 estable (Latest), todas las plataformas + alphas de Switch y Vita; web en Pages.
  v1.9.13-pre2 de prueba (Windows, APK 32/64, Switch GPU) comentada en #22, #33, #34, #35.
- **Rama `claude/perf-android-reloc`** (no está en main; va a main cuando pasen sus dos baterías):
  - J, Android 64 en teléfonos donde 0x01000000 está ocupada (#22 Poco F8 Ultra, #34 Mate XT): la imagen se carga en
    otro hueco y se reubica. En el emulador con `KHCOM_IMAGE_MOVE=1` carga en 0xe2600000 y los menús salen bien.
  - Caja de error con "Share log" (modelo, Android, logs). Si el juego se cae, `kh_com_host.log` trae la señal y la
    cadena de llamadas del juego (direcciones del enlace: `llvm-symbolizer --obj=guest-<región>.macho`).
  - #37 (sin teclado no se salía de "Press..."): un botón del mando, un toque o Atrás cancelan la captura.
  - #38: "ocultar controles táctiles" con un mando conectado no dibuja nada, ni en menús (sin mando queda el botón
    de opciones). Solo Android.
  - I, PPU en varios núcleos (Windows): falta la verificación.
- **21/10 ~17:00, pausadas por espacio:** el emulador reservaba 9 GB y el pagefile creció a 35,7 GB (C: llegó a
  0,3 GB). Android (menús y fixtures con imagen movida: todo ok salvo el selector de eventos, ya arreglado) y la PPU
  (80/194, 0 `MISMATCH`) se retoman con `--resume` cuando haya RAM/disco; la de escritorio sigue. Reiniciar Windows
  devuelve el pagefile. Arreglados después: Event_0 (selector de eventos) y los dos sitios del Bosque de los 100 Acres
  (`BUS_PTR`, `port/src/port_bus.c`), vistos en el informe de escritorio (`scenes` ahora va primero en las baterías).
- **Baterías (estado anterior):**
  - Android, imagen movida, EU menús/fixtures/jefes: `build/android-move-battery.log`,
    `build/android-scenarios/report-eu-khcom_image_move.json`. Relanzar: el mismo comando con `--resume`
    (`build/move_cmd.txt`).
  - PPU: `KHCOM_PPU_VERIFY=1`, eventos US en vista 16:9 (`--wide`; en vista GBA no hay filas fuera de la imagen y la
    verificación no prueba nada): `build/ppuverify-us.log`, `build/ppuverify/us_wide/*/kh_com.log` sin `MISMATCH`.
  - Escritorio (`run_batteries.py --status`): eventos EU hechos, US en curso; luego JP, 16:9 y salas.
- **Protecciones nuevas (21/10):** `check_sources.py` (constantes SDL del shim de consolas con el valor de SDL,
  literales de Java cerrados) corre en `build_android.py` y en `release_check.py`; ya encontró `SDLK_DELETE` mal en el
  shim. El arnés Android ya no da "ok" a un caso que todavía no arrancó, cuenta como caída el `crash:` del host y marca
  `BAD-APK` si el APK de prueba se armó sin `--smoke-env KHCOM_FAST=1` (corría la batalla de prueba en cada caso).

## v2.0 — próxima estable (pedido del usuario 22/10: "stable nueva con 2.0 y anunciar la inclusión del HD")

Es un lanzamiento grande: no publicar hasta pasar TODA la lista de verificación de abajo. PC (Windows, Linux, Steam Deck)
estable; Android, Switch, Vita, PS4, PS5, Anbernic, web como `-beta`. Al final: revisar issues, arreglar sus bugs,
publicar y anunciar el mod HD en las notas. El usuario prueba con `Probar ReKHCoM v1.9.15 completo.bat` (raíz).

Hecho desde la v1.9.14 (en main): efectos que se silenciaban con la música HD (canales del BGM por su `trackCount`),
música HD al volumen del cartucho, carga en hilo aparte, pausa en el lugar (sin tirar la cola), sigue sonando bajo
Opciones, silencio mientras carga; pausa de escena en videos HD (START/Esc; A ya no corta); 97/100 escenas HD
asignadas (`match_hd_scenes.py`); opciones HD en Mejoras de Re:CoM y en el asistente de partida nueva; inicio de
combate sin el cuadro del corazón chico; arte del título difuminado; videos HD con el renderer por software; barras de
vida sin división por cero (#40); salto de cuadros (encendido en todo); Switch #36; Vita en GPU; Discord en Linux.

### Estado 22/10 ~03:00 (traspaso: al Claude anterior se le acabaron los tokens)
- En main: todo lo de arriba + escenas HD 97/100 (`kHdSceneExtras`: 001/614/631 solo en modo Re:CoM), fundidos de
  0,3 s en videos HD (PC y Android), guardas de división (#40 y parientes), flujo de audio HD arreglado.
- Método de depuración y herramientas: **DEBUG_WORKFLOW.md**. Seguirlo.
- Falta: (1) ~~versión v2.0~~ hecho 22/10 (launcher, rc, web, CHANGELOG; tag **v2.0**: el actualizador de la PS5 lee
  "2.0" como 2.0.0 y `build_all.py`/`build_android.py` ya aceptan vX.Y); (2) builds de todas las plataformas desde el commit final;
  (3) la lista de verificación de abajo completa; (4) revisar issues (#40 abierto: arreglado, avisar al publicar);
  (5) publicar v2.0 anunciando el mod HD, borrar releases viejos si el clasificador deja (si no, el usuario).
- Copia del usuario: `Probar ReKHCoM v1.9.15 completo.bat` (actualizarla con cada build de Windows).

### Bugs encontrados y cerrados el 22/10 (todos en main)
- Escena HD: destello de la imagen del evento al terminar (53f756bc7).
- Música del campo bajo el video HD y corte seco; salto sin fundido de imagen y sonido (53f756bc7, PC y Android).
- Pausa del video HD muda (1c8a17352).
- Agrabah 107/108/109 aprobadas con honores pero apagadas para jugadores (d94ce3b56).
- Suelo isométrico: tabla única recalculada por píxel, <5 fps a x1 en 004/057/061/074 (también pesaba en Android,
  web y consolas, un núcleo): 31 ms por cuadro, píxel a píxel igual (e1b635d65).
- Batería: resultados del 21/10 viejos (Event_0, Pooh) ya arreglados; 0 sitios abiertos.
- Pendiente de confirmar: timeout Android US combate 124 (Agrabah) con dos corridas en el mismo emulador; repetir solo.

### Pendientes v2.0 (en orden)
- ~~Arte extendido a x1 en 1080p~~ **hecho 22/10**: a x1 1920x1080 todo lo aprobado ya llega al borde (Guard
  Armor, 004, 008, 057, 061, 074, Agrabah 107/108/109; `build/x1art`). Agrabah 107/108 y 109 (con honores, 21/10)
  estaban detrás de `KHCOM_AGRABAH_WIDE`/`KHCOM_AGRABAH109_WIDE` esperando la integración: ahora encendidas para los
  jugadores (la variable en 0 las apaga para antes/después); `test_agrabah_backdrop.py` 11/11,
  `test_agrabah109_backdrop.py` 10/10 (EU/US/JP, rectángulo GBA idéntico).

1. ~~Escenas HD: disparo real~~ **hecho 22/10**: `test_hd_scenes.py` (cada evento desde el menú de depuración,
   video cortado a 1 s): EU 97/97 en modo original y 97/97 en Re:CoM; 001/614/631 solo en Re:CoM y en su orden. Todos
   los eventos (también los mudos) pasan por `UpdateEventSeq`.
2. ~~Transiciones~~ **hecho 22/10**: al terminar la escena HD el fundido del evento empieza ya en negro (antes volvía
   la imagen del evento a brillo 234 y se fundía 64 cuadros); saltar el video funde imagen y sonido 0,3 s (PC y
   Android); el sonido del juego y la música HD bajan en 150 ms antes del video (antes 200 ms de la música del campo
   sonaban debajo y se cortaban). Medido con el driver "disk" (`build/hdtrans`).
3. ~~358/2 Days~~ **hecho 22/10**: KHDays FM (melonDS, github.com/vitor251093/KHDays_FM) pausa el video HD con
   Saltar/Continuar y sonidos de menú, y baja la música 1 s antes. Tomado: la pausa del video HD suena como los menús
   del juego; la música ya baja antes del video (150 ms, con la pantalla en negro).
4. ~~Consolas~~ **documentado 22/10** en HD_MOD.md (decodificador por consola, qué falta en cada host).
5. Revisar issues abiertos al final, arreglar bugs, responder breve.

### Lista de verificación antes de publicar (no saltear)
- `release_check.py v2.0` antes y `--post` después (versión en launcher, rc, CHANGELOG, web, paquetes, Quiver).
- `check_sources.py` (lo corre build_android y release_check): constantes del shim de consolas.
- Builds: Windows, Linux, Android, PS4, Switch, Vita, PS5 (VM), web — desde el commit final; ningún paquete viejo
  (los scripts de VM ya no reusan paquetes, verificar "fetched" en los logs).
- Combate de prueba con cada ejecutable de PC (`KHCOM_BATTLE=10:4`), sin `bios:`/CRASH.
- `debug_kit.py report` (baterías de escritorio: scenes, events, wide, rooms, battles, riku, monkey, skip) sin
  sitios nuevos.
- Android 64 (`run_android_full.py` / pasos `android ...` de run_batteries): imagen movida + jefes EU/US/JP, Riku.
- `test_hd_audio.py` (ningún efecto tomado por música), `test_hd_audio_flow.py` con `uv run --with numpy --with
  soundfile` (driver "disk": 0 cortes, 0 solapamientos, 0 saltos audibles; incluye Opciones abiertas).
- `test_frameskip.py` (lógica idéntica con salto de cuadros).
- Videos HD: `KHCOM_HDMOVIE_PAUSETEST` (pausa y salto), renderer por software, Android (`KHCOM_MOVIE=1` en el emulador,
  `build/hdtest/run_hdmovie.py` en khcom-claude-hdmov).
- `match_hd_scenes.py --check` (asignaciones consistentes).
- Copia de prueba para el usuario + `.bat`; notas cortas; nunca decir en público que algo no se probó en hardware.

## 21/10 noche (usuario ausente, /goal: bugs y pendientes sin imágenes)

- **v1.9.15-pre1 publicada (prueba: Windows, Switch, Vita)**, comentada en #35 y #36. Incluye: #36 (táctil oculto en
  pausa/Opciones, Minus + Plus 3 s abre Opciones), salto de cuadros (`experimental_frame_skip`, encendido en todo tras
  `test_frameskip.py` 24/24 idénticos y 1/3600 saltados en PC con VSync; RENDIMIENTO.md), Vita en GPU (SDL2 GXM de
  vdpm), cola de audio del puente en unidades de entrada.
- **#36 cerrado** (DrunkECheese probó la pre1: sin quejas).
- **No se pueden cerrar desde acá (21/10 noche):**
  - D. Videos HD en Linux: el paquete es i386 con sus propias librerías Debian; haría falta empaquetar ffmpeg i386
    (libavcodec/libavformat + dependencias, decenas de MB) o portar openh264 + minimp4 + un decodificador AAC.
    Decisión de tamaño/diseño para el usuario. Consolas (sceAvPlayer, nvdec): sin hardware para verificar.
  - I'. PS4 a la GPU (Piglet): sin PS4 ni emulador para verificar; el rasterizador por software sigue, y el salto de
    cuadros ya mantiene la velocidad si no llega.
  - Borrar releases v1.9.12/v1.9.13-pre2 y quitar los worktrees viejos: el clasificador lo bloquea (lo hace el usuario).
- **C. Videos HD en Android: hecho** (RGB por defecto, verificado en el emulador; HD_MOD.md). Rama en main.
- **F. Discord en Linux: hecho.** VM Debian con un socket falso: handshake y SET_ACTIVITY (mundo, piso, nivel, combate).
- #34 cerrado (confirmado). #39 (recortar relleno) respondido: idea, no bug.
- Builds de consolas: `build_ps5.py` enlaza con archivo de respuesta (límite de 32 K) y lee bien los `.d` con rutas de
  Windows; Switch/Vita/PS5 ya no reusan un paquete viejo si la VM falla; Vita rellena 8 KiB si `vita-elf-create` no
  tiene lugar.
- Baterías: `run_batteries.py` suma battles, riku, monkey y skip (EU/US/JP); el AVD se encuentra con ANDROID_AVD_HOME.

## Carpetas de KHCoMPC (21/10, pedido del usuario)

La raíz quedó en: `jugar/` (copias ReKHCoM-vX con partidas del usuario + su .bat), `pruebas/` (copias que esperan
aprobación + .bat), `roms/`, `arte/` (fuentes, imágenes, Showcase, Trailer), `respaldos/`, `toolchain/` y los repos.
Cada una tiene `LEEME.md`. **Codex:** `package_preliminar_v1.py` ahora escribe en `../pruebas/ReKHCoM-preliminarV1`,
`crystal_parity.py` lee `../respaldos/` y el tráiler sale en `../arte/Trailer/` (main); trae main antes de usarlos.

## Reparto

| Quién | Pendiente |
|---|---|
| **Claude** | **J + I a main**: cuando las dos baterías de arriba terminen limpias, build pc-eu/us/jp, prueba de combate, merge a main, APK de prueba. Con el OK del usuario: prerelease y pedir a #22/#34 que prueben; #37/#38 se cierran con la confirmación de xsm2 |
| **Claude** | **A. Batería Android 64 completa**: EU jefes con Riku, US/JP menús y jefes; luego otros perfiles (Android 13/15, tablet, plegable como el Mate XT) y el bot de partida (`ANDROID_64.md`) |
| **Claude** | **B. Batería de escritorio** (`run_batteries.py`): cada `bios:` nuevo se arregla (`KIT_DEPURACION.md`) |
| **Claude** | **C. Videos HD en Android**: rama `claude/android-hd-movies`; el emulador se cuelga en la primera textura YUV. Probar `KHCOM_HDMOVIE_RGB=1` (b4cb4730d) en el emulador; a main solo verificado (`HD_MOD.md`). Luego **D**: consolas y MP4 en Linux |
| **Claude** | **E'. Reportes nuevos (21/10 ~16:15, prioridad del usuario)**: #22 xsm2 Poco F8 Ultra y #34 Mate XT con la pre2: "The game image could not be loaded" = bug J (imagen de 64 bits sin hueco en 0x01000000), arreglado en `claude/perf-android-reloc`; xsm2 no llega a Settings para el log (la rama pone "Share log" en la caja de error). Odin 2 Portal anda bien con la pre2. Casos en la batería: grupo `issues` de `test_android_scenarios.py` (#33 evento 099, Bosque 100 Acres) y paso `android eu move` con `KHCOM_IMAGE_MOVE=1`. `mode_vsbattle` sale de la batería: arnés sin compañero de link (HARNESS_ONLY) |
| **Claude** | **E. Issues**: #33 (Trickmaster, arreglado en pre2) y #35 (Switch lento: si sigue con la GPU, deko3d) esperan a los testers; #36 (Switch se cierra en el título): comentario listo en `build/issue36-comment.md`, sale con la próxima prueba; #25 se cierra cuando el mod HD salga en una estable |
| **Claude** | **G. Integración de Codex**: traer sus commits (`khcom-codex` codex/preliminar-v1 y `khcom`) a main con build y prueba (al día hasta 457d10854) |
| **Claude** | **I'. Rendimiento en consolas**: Vita a la GPU (necesita OK del usuario para `vdpm sdl2` en la VM), PS4 a la GPU (Piglet) (`RENDIMIENTO.md`) |
| **Claude** | **H. Próxima estable**: solo con el OK del usuario; `release_check.py` antes/después, todas las plataformas, web |
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


## Esperando a terceros (no se trabaja)

- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
