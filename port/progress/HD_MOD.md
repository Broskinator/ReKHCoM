# Mod HD: videos y música de KINGDOM HEARTS -HD 1.5+2.5 ReMIX- (21/10/2026, para v1.9.21)

Pedido del usuario: usar el Re:Chain of Memories del 1.5+2.5 de PC que el jugador tenga instalado
(Steam o Epic) para reemplazar los videos de GBA y la música (mod base), y en un mod completo las
escenas de diálogo por las del 1.5, en todos los idiomas. El launcher detecta el juego instalado,
o una carpeta de instalación, o una carpeta con los videos y/o la banda sonora. Opciones on/off
independientes. Primero PC; Android y consolas si se puede. Apoyo: issue #25 (mapa de las 104
escenas a eventos de GBA, de DL-6-case): agradecerle al cerrar.

Nada del juego viaja con ReKHCoM: se lee la instalación del jugador.

## Lo que hay en la instalación (Steam: `steamapps/common/KINGDOM HEARTS -HD 1.5+2.5 ReMIX-`)

- Videos de Re:CoM: `STEAM/dt/KHCReSource/BIN/movie/*.mp4` (también KH1 en `STEAM/dt/KH1Movie`, BBS
  en `STEAM/juefigs/BBSReSource/movie/<idioma>/`). Están ofuscados: **solo los primeros 0x100 bytes**
  van con XOR de una clave fija de 16 bytes `3f1ec193264258b8aa9d5375bf629a97` (posición mod 16); el
  resto es un MP4 normal. ROP (apertura, 111 s), RSE (final de Sora, 353 s), RRE (final de Riku,
  345 s), RLP (157 s) en 1920x1080 H.264 + AAC; `zz_event_###` (100 escenas) en 1280x720.
  Audio en inglés; no hay pistas de otros idiomas.
- Música de Re:CoM: `Image/dt/Recom.hed/.pkg`, archivos `STREAM/0001/*.win32.scd` (45 pistas con
  nombre descriptivo, p. ej. `01F_Town_F`, `19_Alice_Field`). Paquetes: formato de OpenKh (OpenKh.Egs,
  Apache-2.0): `.hed` de entradas MD5(nombre)/offset/tamaños; en el `.pkg` una cabecera de 16 bytes
  que es la semilla de la clave, XOR sobre los primeros 0x100 bytes y zlib.
  Lector propio: `port/tools/khhd_pkg.py` (`list`/`get`). Nombres: `resources/recom.txt` de OpenKh.

## Slots de GBA (include/evt/mode_movie.h)

MOVIE_OPENING → ROP; MOVIE_ENDING → RSE; MOVIE_RIKU_ENDING → RRE; MOVIE_6F_GOAL y MOVIE_12F_E2 →
escenas del 1.5 según #25 (por confirmar mirando). RLP: bucle del menú de título (opcional).

## Diseño

1. **Importar (launcher, Windows primero):** detectar la instalación (bibliotecas de Steam en
   `libraryfolders.vdf`, Epic en sus manifiestos, o carpeta elegida, o carpeta con MP4/OGG ya
   sueltos). Copiar y descifrar a `<datos de ReKHCoM>/hd/movies/*.mp4` (mod base: ROP, RSE, RRE,
   ~1,2 GB; mod completo: las escenas, ~7 GB) y convertir la música a `hd/music/*.ogg`. Lo importado
   es estándar: para Android se copia la carpeta `hd` al teléfono (el launcher de Android puede
   importarla con el selector de carpetas).
2. **Juego:** al pedir un video de GBA, si existe el MP4 importado y la opción está activa, se
   reproduce ese (Windows: Media Foundation; Linux/Android: decodificador a definir) con Start para
   saltar; si no, el video original. La música: tabla canción de GBA → pista de Re:CoM, decodificada
   con stb_vorbis, opción aparte.
3. **Mod completo:** al empezar un evento con escena del 1.5 (mapa de #25), reproducir esa escena en
   lugar del evento, con los subtítulos de Re:CoM en el idioma del juego
   (el audio es inglés). Opción aparte, apagada por defecto.
4. **Opciones (juego + launcher + ini):** `hd_movies`, `hd_music`, `hd_scenes`, independientes.

## Estado

- [x] Videos: ubicación y clave XOR (21/10).
- [x] Paquetes: lector `khhd_pkg.py`, música ubicada.
- [x] Conversión SCD → OGG (`khhd_scd.py`; en el launcher `hd_import.inc`, idéntica byte a byte).
- [x] Importador en el launcher: detecta Steam (registro + libraryfolders.vdf) y Epic (manifiestos),
  o carpeta elegida (instalación o archivos sueltos .mp4/.scd/.ogg); `ReKHCoM.exe --hd-import`.
- [x] Videos HD en el juego (Windows, Media Foundation): apertura, final de Sora, final de Riku.
- [x] Música HD (`port_hdmusic.c`): 43 canciones de GBA → pistas de Re:CoM, sigue pausa/parada/fundidos.
- [x] Mod completo (`hd_scenes`), 22/10: 97 de las 100 escenas asignadas (`match_hd_scenes.py`: diálogo en los
  cinco idiomas del cartucho contra los subtítulos de cada escena, más el orden de las escenas y los pares mudos);
  fuera: zz_event_001 (prólogo, dentro de la apertura que reemplaza ROP), 614 y 631 (solo Re:CoM). Antes: 72 eventos → escenas (filas confiables de #25,
  `port/src/port_hdscenes.inc`); al terminar, el evento se cierra con el salto de escena del juego.
  Audio en inglés (el PC no trae otras pistas).
- [x] Subtítulos (`hd_subtitles`, encendido): sí vienen, en `Recom.pkg` como `FORM/5xxx/FO5xxx.BIN`
  (tabla MVS en inglés US + remasterizadas UK/FR/GR/IT/SP). `khhd_subs.py` y el launcher escriben
  `hd/subs/<video>.<idioma>.txt` (581 archivos, idénticos byte a byte); el juego los dibuja bajo la
  imagen en su idioma (KHMenu, contorno oscuro). Algunas tablas descomprimen más de lo que dice su
  cabecera: se conserva todo (21/10).
- [x] Reproducción fluida: el video sale en NV12 del decodificador y la GPU lo convierte (textura NV12
  BT.709). Pedir RGB a Media Foundation hacía la conversión por CPU y los videos iban lentísimo. El
  plano de color se ubica por las filas reales del buffer (1088 en H.264 1080p); con 1080 el color
  quedaba corrido hacia abajo (reporte del usuario, 21/10).
- [x] Opciones en el juego (filas visibles solo si se importó) y en el launcher.
- [x] Música HD en las imágenes con puente (Android 64 bits, Switch, Vita, PS4, PS5), 21/10: la imagen
  no tiene libc para stb_vorbis, así que `port_hdmusic.c` conserva la lógica del juego (qué canción,
  pausa, fundido, ganancia) y en esas imágenes llama a la entrada nueva del puente `hd_music(op, ruta,
  ganancia/1000)` (0 abrir, 1 sonar, 2 pausa, 3 cerrar). Cada host incluye
  `port/ps5/shared/hd_music_host.h`: lee el .ogg/.loop con sus propios H_fopen/H_fread, decodifica con
  stb_vorbis y mezcla la pista en cada bloque que el juego empuja (`H_audio_push` = `HdMusicPush` sobre
  el push original, que ahora se llama `GamePush`), remuestreando lineal de 48 kHz a la tasa del juego.
  Sin hilos: sigue el reloj de audio del juego. PC y Android 32 bits siguen con su stream SDL propio.
  Una pista que no abre se intenta una sola vez por canción (antes se releía el archivo cada cuadro).
  Compilado: PC, APK (arm64 + armv7), host PS4. Probado en el emulador Android 14 con el host de 64
  bits (`test_android_scenarios.py --groups battles --only battle-000- --slow 6`, con
  `files/hd/music/01F_Town_B.ogg/.loop`: el combate de Ciudad de Paso pide esa pista): abre a 48 kHz,
  "song 12 -> 01F_Town_B", termina sin fallos (21/10). Switch (.nro) y Vita (.vpk) compilan en la VM (21/10).
- [x] Android: botón "Importar mod HD" en el launcher (selector de carpeta SAF; se elige la carpeta `hd`
  del launcher de PC o la que la contiene) que copia `hd/music` (.ogg/.loop) a `files/hd/music`, cada
  archivo entero (temporal + renombrar). Los videos todavía no: Android no los reproduce y pesan GB.
  Probado en el emulador Android 14: Descargas/hd → 2 pistas, idénticas por md5 (21/10). Android no deja
  elegir la raíz de Descargas: hay que elegir `hd` (el texto del botón lo dice).
- [x] Videos en Android (32 y 64 bits), 21/10: `port/portable/hdmovie_android.c` (MediaExtractor + MediaCodec a un
  AImageReader, subtítulos con FreeType, dlopen: Android 7+), entrada `hd_movie` del puente en el host de 64 bits, el
  importador copia videos y subtítulos. La imagen va a RGB en la CPU (tablas, un par de píxeles por croma) y se sube
  como textura común: las texturas YUV de SDL colgaban el emulador al crear su shader, así que quedan solo con
  `KHCOM_HDMOVIE_YUV=1`. Verificado en el emulador Android 14 (host de 64 bits): la apertura ROP 1080p completa con
  subtítulos, colores correctos (`KHCOM_HDMOVIE_SHOT=20`); el emulador traduce ARM y descarta cuadros, un teléfono no.
  Prueba: `KHCOM_MOVIE=1` con `files/hd/movies/ROP.mp4`.
- [ ] Videos en consolas: decodificador de cada una (Vita/PS4 sceAvPlayer, Switch nvdec); sin hardware
  para probar, después de Android.
- [ ] Linux: reproducción de MP4 (la música HD ya funciona en Linux).

## Otros pedidos para v1.9.21 (anotados acá para no perderlos)

- Texto HD: que la "a" minúscula no se confunda con la "o" y que la "r" se note más.
- Android de 64 bits y batería: `ANDROID_64.md`.
- Al final: todos los issues abiertos resueltos y respondidos (#22, #25, #33, #34…).
