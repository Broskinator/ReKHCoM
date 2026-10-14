# Reparto de pendientes — reordenado el 14 de octubre de 2026

Responsabilidades acordadas con el usuario para trabajar sin solaparse,
ordenadas por las barras de `status.json` que faltan para el 100 %.
`[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## Barras pendientes y dueño

| Barra | % | Peso | Dueño | Qué falta para el 100 % |
|---|---|---|---|---|
| Gameplay verification | 66 | 10 | Claude | Recorrido de todas las salas, estrés de submenús, partida completa Sora y Reverse/Rebirth |
| Widescreen: field maps | 85 | 8 | Codex | Salas con huecos o bordes de abismo; revisión por mundo |
| HD text | 88 | 3 | Claude | Inventario de números/textos en pixel art, oclusión general |
| Widescreen: battles | 93 | 6 | Codex | Cinta continua de arenas y bordes de abismo |
| Widescreen: menus & events | 93 | 4 | Codex | Salida de submenús, footer, subida de nivel (paneles y popeo) |
| Graphics options menu | 95 | 5 | Claude | Prueba de cada opción en vivo y en los tres idiomas de cartucho |
| PPU renderer | 97 | 10 | Claude (cierre) + Codex (presentación) | Regresión de conformidad; los cambios de composición se coordinan |

Ya al 100 %: build, assets, memoria, BIOS, DMA, interrupciones, entrada, m4a,
PSG, PCM, códec de bloques, películas, SRAM, reset, widescreen renderer,
estados guardados, launcher, saltar intro, versiones USA y japonesa.

## Codex: acabado panorámico (field, battles, menus & events)

- [x] Cristal del castillo: desaparece tras abrir/cerrar repetidamente su menú.
  Corregida la sustitución de tareas retenidas en salas fijas. Doce ciclos del
  menú conservan una sola tarea y el cristal visible; añadir uso real de A/B
  a la batería ampliada de navegación.
- [x] Cian en selección de carta de puerta (estado 7) y animación posterior
  de apertura (estado 9). El spotlight delimita su canvas y conserva márgenes
  negros; fixtures de spotlight/selección pasan sin cian en márgenes.
- [x] Entrada a combate: fondo difuminado como en las películas, animación
  central en su proporción. Revisado en EU/US/JP (test_levelup.py --entry).
- [~] Headers de Ver barajas, Añadir/Quitar, Naipes mapa, Estado/magias,
  Mapamundi, Naipes mundo y Diario desde los elementos originales, sin tomar
  columnas de la imagen final. Muestreo de una pieza limpia del asset original;
  Estado y barajas capturados; completar recorrido visual y cursor arriba/abajo.
- [~] Tabs del ojo a izquierda y derecha: relleno y contorno propios desde el
  tilemap/fotograma del tab. Completar captura de cada pestaña y transición.
- [~] Naipes mundo: fundido de pixel art tramado desde la paleta original.
  Revisado en EU (build/priority-regressions/20261013_231639_172982/world);
  capturas con partidas US/JP pendientes.
- [ ] Entradas/salidas de submenús sin ecos ni distorsión; Mapamundi con footer
  completo desde el inicio y al volver. Barras animadas de Mapamundi desde sus
  assets OBJ, revisadas en EU (build/priority-regressions/20261013_232146_939954/allmap).
  Otros submenús y ejecución regional pendientes.
- [ ] Subida de nivel: prolongar relleno/sombra/contorno de los paneles derechos
  hasta el borde y verificar el popeo en una entrada natural. Franjas unidas en
  EU (build/levelup-check/20261013_230113, build/levelup-reference/20261013_230037).
- [ ] Sustituir el estirado de píxeles por patrones o piezas de fondo donde sea
  viable; conservar los patrones aprobados de barajas y Mapamundi; revisar el
  píxel residual del margen derecho.
- [ ] Arenas como cinta circular de tiles: clasificar fondos repetibles y con
  elementos únicos; no duplicar jefes ni sprites ni cambiar colisiones.
- [ ] Widescreen field: recorrer por mundo las salas con huecos o abismo usando
  la batería de salas de Claude (abajo) y corregir lo que aparezca.
- [ ] Al cerrar cada barra: actualizar evidencia y porcentaje en status.json.

## Claude: verificación, regiones, texto HD, opciones y cierre del PPU

- [ ] Gameplay verification: batería de todas las salas de cada mundo y planta
  con el modo MAP del menú de depuración (mode_mapchk), sin crashes y con
  capturas para la revisión panorámica de Codex.
- [ ] Gameplay verification: ampliar la prueba de crashes al alternar
  submenús/edición (Estado, Diario, Naipes mapa, Añadir/Quitar) y comprobar
  que los avisos de tutorial quedan delante del texto de las cartas.
  (Pasada de Codex a Claude el 14/10 para acelerar.)
- [ ] Gameplay verification: medir el stutter al permanecer en la subida de
  nivel con tiempos de fotograma y lógica, a ritmo normal; corregir si existe.
  (Pasada de Codex a Claude el 14/10.)
- [ ] Gameplay verification: partida completa de Sora y de Reverse/Rebirth en
  las tres regiones (rutas reproducibles y problemas anotados).
- [x] USA/JP: combates JP 209/209 como Sora y como Riku; eventos 192/194 (los dos
  restantes se cortaron al parar, no fallaron). Barra al 100 %.
- [ ] USA/JP: capturas regionales de los acabados panorámicos que Codex marque
  como revisados solo en EU.
- [ ] HD text: inventario de números y textos aún en pixel art, reemplazo
  opcional de los que son texto; regresión general de oclusión. Coordinar con
  Codex antes de tocar prioridades/oclusión del compositor.
- [ ] Graphics options menu: prueba automática de cada opción en vivo y del
  menú en EU (cinco idiomas), US y JP.
- [ ] PPU: regresión de conformidad (capturas de referencia por modo y efecto)
  para cerrar la barra; cambios de composición coordinados con Codex.
- [ ] Preservación: reproducir los casos de BUGS_GBA.md en EU/US/JP y separar
  técnicas originales, anomalías del código y defectos del port.
- [x] Launcher e instalación: batería 11/11.
- [x] Audio: comparación con emulador (música, vídeos), batería de las 637
  canciones (port/progress/songs_report.txt), detalles del PSG, audio suave.
- [x] Estados guardados deterministas (3/3).
- [x] Releases v0.5, v0.6 y v0.7 (pre-releases públicas).
- [~] QoL: devolver cartas acumuladas y bancos de partidas hechos; aviso de
  puntos moguri, revancha de jefes y repetir tutoriales pendientes (no suman
  barras; después de las barras).
- [~] HD-2D experimental (versión alternativa): profundidad de campo, bloom y
  gradación publicados; el pop-up de diorama está en un stash (bded017)
  y vuelve después de las barras.

## Reglas compartidas

- No modificar a la vez los mismos archivos; Codex trabaja principalmente en
  map/map_tasks/map_cell, card_level_up, perfiles de menú, PPU y presentación.
  Claude trabaja en harness y pruebas, opciones, launcher, audio y texto HD.
- Cada uno hace commit solo de sus propios cambios (por bloques si un archivo es
  compartido) y no toca el trabajo sin commitear del otro.
- No sobreescribir ejecutables o estados que el usuario esté usando. La build
  compatible con sus estados está en build/priority-reference; las ranuras
  3/4/7/8/9 recientes en build/pcwork/states.
- Probar en directorios aislados y con pocos procesos mientras el usuario juega.
  No modificar partidas/configuración del usuario.
- No subir ROM ni recursos extraídos al repositorio.
- Subir porcentajes solo con evidencia y distinguir implementado de verificado.
- Usar emuladores únicamente para contrastar bugs o comportamientos dudosos del
  original. Validar acabados visuales y transiciones directamente en el port.
