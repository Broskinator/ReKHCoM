# Tráiler de ReKHCoM — cómo se hizo y qué falta (v4)

Herramientas: el propio juego (grabación y audio), ffmpeg 9 y Python+Pillow.
Sin editor de vídeo ni IA. Todo está en `../Trailer/` (fuera del repo):

- `Trailer/tools/` scripts (antes vivían en el scratchpad de Claude):
  - `fx.py` arnés: copia `build/<build>/khcom.exe` a `build/fx_<nombre>/`, escribe
    `kh_com.ini`, lanza con drivers dummy y sin mandos (SDL_JOYSTICK_*=0, para no
    leer el mando del usuario), y devuelve los BMP de `KHCOM_SHOTS`.
    `COPYSTATE=<.khst>` copia un estado y se carga con `KHCOM_LOADSTATE`.
  - `trailer_rec.py` graba cada clip con `KHCOM_RECORD=desde-hasta:ruta.mov`
    (ProRes 1080p60 por ffmpeg, lee la ventana). Lista `jobs`: idioma, diario,
    entrada a combate, 9 arenas (`KHCOM_BATTLE=10:<arena>:0` + `KHCOM_MONKEY`
    para que Sora pegue al azar), salas por mundo (`room:1WW01` andando UP),
    sala de Monstruo saltando el barril (`05_sala_mundo_3_salto`), 32:9
    (Agrabah arriba, Halloween abajo, ventana 2560x720), película, tienda
    texto píxel/HD, idiomas, castillo 4:3 y 16:9 (`00c`/`00d`), menú de
    opciones (`15_menu_opciones` con `KHCOM_VPAD`).
  - `sfx_rec.py` re-graba cada clip solo con efectos (música a 0) y
    `KHCOM_WAVDUMP`; los WAV van a `Trailer/audio/sfx/`. La música va aparte
    (`Trailer/audio/m_titulo.wav`, `m_combate_agrabah.wav`).
  - `cards.py` títulos (Palatino, brillo, línea dorada) -> `tools/cards/`.
  - `sweep.py` barrido texto ORIGINAL/HD; `console.py` dibuja la consola
    genérica (sin marca); `intro.py` monta la intro de la consola;
    `postcred.py` post-créditos Sincorazón "ANDROID RELEASE / COMING SOON"
    (ya hecho: `clips/16_postcreditos_android.mov`).
  - `edit.py` motor de montaje (xfade, transición corazón `heart_expr.txt`,
    slider `slider_expr.txt`, zoom/push-in, tarjetas `card`/`card2`),
    `acts.py` los actos A..F (v4), `final.py` une los actos y mezcla música
    + efectos con loudnorm -14 LUFS. `acts_v3.py`/`final_v3.py` = la v3.
- Salida: `Trailer/ReKHCoM_trailer_v4.mp4` (v3 = `_final.mp4`, publicada).

Orden: build release actual -> `python trailer_rec.py` -> `python
sfx_rec.py` -> `python intro.py` (necesita 00c/00d) -> `python sweep.py`
-> `python cards.py` -> `python acts.py A B D E F` -> `python final.py`.
Revisa con hojas de contacto (ffmpeg select+tile), 3-4 por tramo, no más.

## Mejoras pedidas por el usuario para la v4 (pendientes)

1. Grabar TODO con la versión publicada más nueva (v1.8.2), zoom x4.
2. Intro: escena del Castillo del Olvido (blanco, contraste) dentro de la
   consola (`cards/console.png`, hueco de pantalla 480x320 en 720,350).
   Empieza con "2004 · Game Boy Advance"; push-in rápido hasta llenar la
   altura; luego el SLIDER (como el de texto ORIGINAL/HD) abre de 4:3 a 16:9
   a pantalla completa; quedarse >=3 s en 16:9 con "2026 · Native on PC".
3. La música empieza justo cuando pasa a 16:9 (T_REVEAL en `final.py`) y
   sigue hasta el nombre del proyecto, como antes.
4. Salas: progresión encadenada — cortar de una sala a la siguiente con
   Sora en la misma fase del movimiento (p.ej. a media escalera en una y a
   media escalera en la otra). Monstruo: usar el clip saltando el barril.
5. Final como la v3 (logo, disclaimer, música que se apaga) + post-créditos
   de los Sincorazón (ojos amarillos, emblema, "ANDROID RELEASE / COMING
   SOON"), sin música.
6. Mostrar el menú de opciones interno (clip 15) y el launcher.
7. Sin capturas sueltas (nada de World Warp ni frames congelados de la
   película al final). Transiciones como la v3 (destellos, corazón, slider).
