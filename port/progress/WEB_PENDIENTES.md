# Navegador — pendientes y evidencia

Actualizado: 16/10/2026. Prioridad actual del usuario: navegador en GitHub Pages.
Versión: alpha independiente de la estable Windows v1.8.3. No sustituir la
estable ni publicar la futura PC v1.9 sin cerrar sus verificaciones.

## Hecho / observado

- [x] Compilar el juego C real a WebAssembly (EU, US, JP), no un launcher vacío.
- [x] Loader de ROM local con región, tamaño y SHA-1; sin subir ni empaquetar ROM.
- [x] Reutilizar renderizador, audio, zoom, menús y optimizaciones de PC/Android.
- [x] Arranque EU y diálogos de introducción observados por el usuario en Chrome.
- [x] Usuario confirma texto SD correcto en diálogo de Donald.
- [x] Corregir acceso JS a HEAPU8: export explícito en Emscripten 6.
- [x] Corregir declaraciones C implícitas que generaban firmas Wasm incompatibles.
- [x] Forzar texto original en cada frame Web, como Android. Re:Chain podía activar
  hdText después del arranque y dejar cuadros vacíos con el backend HD stub.
- [x] Evitar duplicar ROM en MEMFS; canvas sin multiplicación por DPR; espera sin
  busy-spin; no segundo timer de SDL RenderPresent; overlay táctil opcional.
- [x] Implementar SRAM/config en IndexedDB, importación/exportación .sav y pausa
  al ocultar pestaña. Implementado no equivale a verificado: ver pruebas abajo.

## Publicación inmediata

- [x] Recompilar los tres módulos con la protección SD final; cero avisos de firma.
- [x] Prueba ejecutable ABI Wasm/Asyncify con memoria fija y callback convertido.
- [x] Publicar rama gh-pages con allowlist; conservar ROM/SRAM/config privados.
- [x] Verificar URL pública y añadir enlace al README y al About del repositorio.
- [x] Registrar commit, hashes, tamaño y evidencia exacta del despliegue.

Publicación verificada: https://matymeatboy.github.io/ReKHCom/ .
Evidencia: WEB_PUBLICACION.json (12/12 hashes y tipos MIME correctos).

## Prioridad alta: compatibilidad y partidas

- [ ] Probar en la URL pública: seleccionar ROM, empezar, diálogo, combate y menús.
- [ ] Probar US y JP en navegador; compilar no garantiza juego regional correcto.
- [ ] Importar una SRAM existente, cargar, guardar en juego, recargar, volver a
  seleccionar ROM y verificar persistencia y contenido de la partida.
- [ ] Exportar .sav y comparar con SRAM del motor; verificar nombres/bancos EU/US/JP.
- [ ] Manejar IndexedDB bloqueado/cuota/privado con aviso preciso y exportación.
- [ ] Probar exportación del banco activo (actualmente botón exporta banco 1).
- [ ] Reset/salida: confirmar sync completo antes de recargar; evitar perder un
  guardado reciente por temporizador o cierre inmediato de pestaña.
- [ ] Pausa/resume de pestaña sin inputs atascados ni ráfaga de audio atrasado.

## Texto HD portátil

- [ ] Crear backend portátil para fuentes; el actual usa GDI de Windows.
- [ ] Mantener métricas, anchos, baseline, colores/fades y ocultación de cada capa.
- [ ] Diálogos, subtítulos, menús, acentos y japonés; EXP sigue pixel como pidió usuario.
- [ ] Verificar Ver barajas / añadir-quitar naipes y animaciones, sin solapamiento.
- [ ] Caché de glifos limitada; liberar recursos al cambiar escala/contexto.
- [ ] Activar opción HD únicamente cuando el backend pueda dibujar el texto.

## Audio y rendimiento

- [ ] Medir FPS, tiempos PPU/present, memoria y latencia en Chrome/Firefox/Safari.
- [ ] Medir x1 como benchmark y x4 por defecto; no prometer 60 FPS por compilar.
- [ ] Comprobar buzz, saltos, pausa de AudioContext, reconexión y cambio de salida.
- [ ] Medir coste de Asyncify/compatibilidad de function pointers; optimizar con
  evidencia. No eliminar el soporte a ciegas: películas tienen bucle propio.
- [ ] Evaluar reducir reserva de memoria manteniendo compatibilidad de punteros.
  256 MiB de espacio lineal inicial no son una medida de RAM residente.
- [ ] Evaluar Worker/OffscreenCanvas para UI fluida; threads solo si aportan una
  mejora medida. Multihilo requiere revisar COOP/COEP y alojamiento.
- [ ] No dejar logs crecer sin límite ni hacer sync/compilar shaders cada frame.

## Móvil y controles

- [ ] Android Chrome de gama baja e iOS Safari físicos: memoria, audio y temperatura.
- [ ] Safe areas, orientación, pantalla completa, redimensionado y zoom del navegador.
- [ ] Botones con tamaño táctil cómodo, diagonales y varios dedos sin bloquear scroll
  fuera del canvas; revisar silueta de rodar y símbolo de naipes.
- [ ] Mandos: conexión, desconexión, remapeo y pérdida de foco.
- [ ] Accesibilidad del launcher: teclado, lector de pantalla y estado de errores.

## Distribución y mantenimiento

- [x] Integrar Web en build_all.py para compilar todas las regiones con un comando.
- [ ] Versionar módulos por hash para evitar mezclar JS/Wasm viejos en caché.
- [ ] Empaquetado reproducible y publicación automatizada sin ROMs privadas.
- [ ] Evaluar descarga offline/PWA después de validar guardados y actualizaciones.
- [ ] Capturas públicas Web actuales, documentación para jugadores y changelog.
- [ ] Mantener licencia SDL/créditos; sin énfasis promocional en herramientas IA.

## PC / Android siguen pendientes

Consultar RELEASE_V19.md y REPARTO.md. Recompilar y verificar PC tras guardas de
stock US/JP y prototipos; preparar v1.9 sin regresiones. Android ARM alpha está
compilada y x86 emulado probado; falta dispositivo ARM físico. No afirmar prueba
de ARM ni texto HD portátil. Conservar ROMs, partidas y savestates del usuario.
