# Entrega del tráiler v4 — v1.8.2

Vídeo local: ../Trailer/ReKHCoM_trailer_v4.mp4 (también en v182/).
No se ha subido a YouTube. La release del juego continúa pública con su ZIP
verificado; el vídeo queda disponible para revisión del usuario.

- 1920x1080, H.264, 60 fps, 71,967 segundos; AAC estéreo a 48 kHz.
- 60,116,521 bytes; SHA-256 e74ec6e6ca240aec8069587ef12467168041e7914b2e2abd3880c40a86f57ebc.
- Decodificación completa sin errores. Audio y vídeo difieren 0,023 segundos.
- Silencio medido al inicio y en los poscréditos: pico/RMS = 0.
- Música activa tras el slider: RMS 0,0400 en 4,1–5,1s.
- Comparación del vídeo final con el mismo instante del paseo: error RGB medio
  2,90 a 4,2s y 3,89 a 7s (tolerancia 8 por compresión/chroma).
- 32 ejecutables de captura coinciden con los tres binarios regionales del
  ZIP público. Material nuevo separado en ../Trailer/v182; anteriores intactos.
- Intro 240x160 -> altura completa -> slider -> x4. Recorrido acotado para
  permanecer dentro del castillo. Mundos encadenados en la fase de escalera,
  salto del barril de Monstruo, dos combates 32:9, texto HD/original, seis
  idiomas, opciones internas y launcher v1.8.2, cierre y poscréditos Android.
- Descripción de YouTube con capítulos actualizados en ../Trailer/.

## Evitar el desfase del slider

xfade empieza B en su instante local cero cuando A llega al offset. Usar la
misma cámara no basta: para un offset 3,8s, B debe empezar en el frame de
fuente 3,8s (`trim=start=3.8,setpts=PTS-STARTPTS`). Su zoom usa t+3,8 para
seguir el reloj global. verify_final.py compara fuente y exportación a 4,2s
(durante el barrido) y 7s (x4 completo). El primer intento falló en la revisión
visual y no se publicó. Se corrigió antes de entregar.

La composición normal desde cero está en port/tools/trailer/. Para acelerar
esta corrección local se sustituyeron los primeros 803 frames del primer
montaje; la transición de corazón y el resto ya revisado se conservaron,
con audio idéntico. El export final volvió a pasar toda la verificación.
