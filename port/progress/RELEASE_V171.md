# Integración v1.7.1 — 16/10/2026

Base pública v1.7 de Claude: b328bc322. El usuario pidió integrar y publicar
cuando Claude agotó sus créditos. Se conservan sus arreglos de interfaz.

## Cambios y evidencia

- PPU: integración de la rama codex/ppu-performance. Decodificación de tiles
  por tramos y composición compartida con oclusión de texto. Ver
  CODEX_PPU_RENDIMIENTO.md y los commits 124ab115d, 878fb3f34, c96f0ec91.
- Audio: ver AUDIO_BUZZ_V171.md. Ranura antigua reproducida con el ejecutable
  compatible; 24/24 límites PCM y 800.000 integrales PSG sin error.
  Arranque/carga preparan la cola antes de reproducir. La escucha del usuario
  sigue siendo necesaria para confirmar si desaparece su zumbido percibido.
- Temporizador: quitar la espera activa adicional de 1 ms. Comparación
  controlada de 360 frames, mismo lienzo fijo 320×180 y SDL dummy: CPU del
  proceso 1,890625 s → 1,328125 s (aprox. 30 % menos), ambos 59,73 fps estables.
  Es una medición de esta escena y equipo, no una promesa para todo el juego.

## Regresión detectada durante revisión

El intento de usar gPortFrame directamente como imagen de presentación provocó
capturas diferentes en combate y tras abrir opciones a 5120×1440. El mismo
binario público repetido en directorios aislados dio imágenes idénticas; al
restaurar la copia independiente desaparecieron ambas diferencias. Se retira
esa optimización. No reintroducir alias de los buffers sin demostrar equivalencia
visual en combate, pausa, opciones y transiciones ultrawide. No asumir que la
prueba PPU aislada demuestra la corrección de la presentación completa.

Las capturas anteriores al cambio solicitado de zoom coinciden en campo,
combate, estado y síntesis a 16:9; a 32:9 también coinciden tras retirar el alias.
La nueva numeración cambia el lienzo Auto por pedido explícito. Se comparan
además escenas a lienzo fijo contra el ZIP público v1.7 para aislar el renderer.
Informes locales: build/v171-review/{visual,always-copy,final,auto-final}.

PPU oracle: 104/107; tres casos históricos de oclusión semántica HD HUD
(cover_bg, cover_obj y window) ya fallaban en la referencia. Las pruebas de
píxeles de hardware pasan. No declarar ese suite 107/107.

## Zoom: especificación final del usuario

La última instrucción pide la numeración antigua, sin decimales, y separar
enteras de intermedias. Sustituye la propuesta anterior de numerar el campo de
visión como x1/x2/x3 y de nombrar pasos «Cercana»/«Lejana».

**x4 de v1.4 sí era una escala entera de píxeles**: cuatro píxeles de pantalla
por cada píxel GBA. A 1920×1080 el lienzo es exactamente 480×270. Su campo visible
vertical equivale a 270/160 = 1,6875 alturas GBA; esa proporción NO es la escala
de píxeles y no se usa como etiqueta. El preset clásico replica la cuarta
fórmula de v1.4; a 5120×1440 corresponde a x5, lienzo 1024×288.

- Predeterminado zoom=0: preset clásico v1.4. Las resoluciones usan su regla
  original de pasos distintos. Se mantiene la adaptación al cambiar ventana.
- Solo escalas enteras activado: aumentos de píxeles xN reales, sin redondear
  silenciosamente una elección fraccionaria ni repetir opciones.
- Desactivado: añade medios pasos, por ejemplo «x4-x5 (intermedia)».
  No hay decimales en las etiquetas; el valor real es la mitad entre ambos.
- Reducir el número muestra más escenario y cuesta más CPU. Más RAM no reduce
  el trabajo por píxel del PPU. Los pasos exigentes se siguen marcando.
- Config v9 migra una vez a la clásica. En v9 se conservan las preferencias.
  IDs nuevos: 0 clásico, el resto es el doble del aumento real; pares enteros.
  Launcher y juego comparten port_zoom.h para listas y filtro.
- `python port/tools/test_zoom.py`: 116 casos de resolución/zoom, límites,
  filtro, orden sin duplicados, equivalencia antigua y persistencia/migración.

## Publicación

Compilar EU/US/JP y launcher, empaquetar sin ROM, verificar ZIP y SHA256.
Preparar acceso local sin sobrescribir partidas ni estados antiguos. Los quick
states son específicos del ejecutable; .sav normales compatibles.

Después de comprobar la nueva release pública, retirar los releases del lote
solicitado, conservando todos sus tags. El pedido comenzó el 15/10 en Santiago:
se conserva el plan exacto en build/v171-review/release-cleanup-plan.json y la
respuesta original de GitHub en releases-before.json. Los releases más antiguos
estaban ocultos como drafts y se mantienen así. No borrar tags ni partidas.

Cierre de QA: seis casos Auto (campo/combate 16:9 y 32:9, media escala y
filtro entero), síntesis/estado/combate contra v1.7 a lienzo fijo idénticos.
Capturas de menús revisadas: x4 legible; fondo de arena continúa sin negro
vacío inferior. EU/US/JP y guardar/cargar estado nuevo pasan.

Último ajuste solicitado: no mostrar la procedencia v1.4 en opciones/ayuda.
Las etiquetas visibles son xN o xN-xM (intermedia); la equivalencia es interna.
