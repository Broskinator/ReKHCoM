# Escena 061: configuración recuperada con el usuario

**Estado: ACEPTADA, PERO MEJORABLE — 18/10/2026.** El usuario pide commitear
primero esta base, para que main la integre, y luego ensayar mejoras por separado.
Pendientes visuales: detalle/sombra extraña en la pared arriba a la derecha;
motivo del suelo arriba a la izquierda con contorno de cuadrado isométrico
incompleto. Ninguna mejora posterior sustituye esta base sin revisión.

**Actualización: MEJORA ACEPTADA por el usuario.** Receta nueva:
`py -3 port/tools/scene061_polish_preview.py`. Su salida
`polish061/061-approved-polish-native.png` coincide píxel por píxel con la
prueba mejorada revisada. La base previa sigue preservada en `a3ff671f6`.

- Suelo: destino BG3 x=-5..3, y=3..7; copiar entrada original (x+12,y+6).
  Completa el rombo atravesando el borde anterior. En esa región, BG2 toma
  la entrada transparente original (0,3) para evitar superponer el motivo viejo.
- Pared: destinos BG2 (32..36,5) toman respectivamente las entradas originales
  (16,0), (17,0), (2,0), (3,0), (16,0). Conservan exactamente los píxeles claros
  y sustituyen la franja de sombra por la textura azul de panel original.
- La mejora cambia 1023 píxeles frente a la base, incluidos 162 dentro del
  encuadre GBA original al completar el rombo. No se inventan colores ni tiles.
- El header de integración ya contiene ambos ajustes. Main debe aplicar el
  perfil también dentro del mapa (motivo que cruza el borde), usando siempre
  mapas fuente originales sin mutarlos. Todavía no se ha conectado al PPU aquí.

Base privada congelada antes de las mejoras: `build/scene-review/accepted061/`.
SHA256 del PNG nativo aceptado:
`e1525b9d8d89281528a57ba5c41c3803aa869f25c968f61cda3eb9526bf16e24`.

Para integración por main: `port/include/port/port_scene061_tiles.h` contiene
`PortScene061TileSource`. Usarlo solo para evento 061, en BG3/BG2, al resolver
coordenadas fuera del mapa. Después, leer directamente el tile de la coordenada
devuelta; omitir el filtro genérico que salta decoraciones. Las capas deben
seguir habilitadas por los bordes L/R/T/B + UPPER. No extender este perfil a 192
sin revisar esa escena. El header aún no está conectado a `port_ppu.c` en este
commit: main está integrando y ese archivo tenía cambios ajenos en curso.

El usuario eligió la distribución de la prueba que se había generado con
`weighted_iso_entry` y extensión de BG3 + BG2. La captura que adjuntó sirve de
referencia visual; no debe usarse como textura ni repararse con clonación de
píxeles. El resultado se reconstruye desde los tiles, mapas y paletas de la ROM.

Receta guardada: `port/tools/scene061_tile_preview.py`.
Ejecución: `py -3 port/tools/scene061_tile_preview.py`.
Salidas privadas: `build/scene-review/recovered061/`.

Pasos fijados, en tiles GBA de 8×8:

| Capa | Izquierda | Derecha | Arriba | Abajo |
| --- | --- | --- | --- | --- |
| BG3 (suelo) | +4,+2 | -4,+2 | -4,+2 | -4,-2 |
| BG2 (pared) | +12,+6 | -12,-6 | +12,+6 | -4,-2 |

Son los valores impresos por la prueba anterior, no una regla nueva aplicada
a todas las escenas. No sustituir la pared por el período del suelo ni volver
a seleccionar automáticamente otra distribución al regenerar esta muestra.

Encuadre: evento 061, captura `run1/0_061/shot_01000.bmp`, cámara GBA (14,55),
vista 320×180. La receta conserva los personajes de las capturas privadas y
rellena únicamente los píxeles negros que quedan fuera del mapa original.

`061-recovered.png` reproduce la configuración anterior. `061-corner-fixed.png`
conserva esos pasos y corrige el ciclo de la esquina: al cruzar el borde derecho,
el paso de arriba puede volver a sacar el punto por la derecha. Se refleja solo
su componente horizontal, manteniendo el período y copiando un tile original.
Cambian 78 píxeles nativos entre (284,0) y (319,2); el resto coincide.
Detalles exactos en `configuration.json`. No se dibujaron píxeles sobre la captura.

La receta es un estudio reproducible. No modifica `port_ppu.c`, la lista de
escenas ni los ejecutables. El trabajo de Claude que ya estaba en ese archivo
se conserva.
