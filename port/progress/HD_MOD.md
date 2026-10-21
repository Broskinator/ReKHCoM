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
   lugar del evento, con subtítulos en el idioma del juego tomados de los textos del propio evento
   (el audio es inglés). Opción aparte, apagada por defecto.
4. **Opciones (juego + launcher + ini):** `hd_movies`, `hd_music`, `hd_scenes`, independientes.

## Estado

- [x] Videos: ubicación y clave XOR (21/10).
- [x] Paquetes: lector `khhd_pkg.py`, música ubicada.
- [ ] Conversión SCD → OGG.
- [ ] Importador en el launcher (+ detección Steam/Epic/carpeta).
- [ ] Reproducción de MP4 en el juego (Windows) y opción `hd_movies`.
- [ ] Música HD y opción `hd_music` (tabla de canciones).
- [ ] Mod completo (`hd_scenes`), subtítulos por idioma.
- [ ] Android: importar la carpeta `hd`; Linux.

## Otros pedidos para v1.9.21 (anotados acá para no perderlos)

- Texto HD: que la "a" minúscula no se confunda con la "o" y que la "r" se note más.
- Android de 64 bits y batería: `ANDROID_64.md`.
- Al final: todos los issues abiertos resueltos y respondidos (#22, #25, #33, #34…).
