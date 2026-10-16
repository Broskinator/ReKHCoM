# v1.8.2 republicada — continuidad

El usuario pide a Codex terminar los bugs A-C y republicar v1.8.2 al agotarse
los créditos de Claude. Esta autorización sustituye el bloqueo histórico de
publicación. El submenú de barajas HD sigue aplazado según el último reparto;
no describirlo como completamente resuelto.

## Causas y solución

- A: `gPortMenuBackground` solo se componía para menús. El presentador lo
  consultaba también en pantallas estrechas y frames sin BG; podía leer el
  blanco antiguo tras dibujar el centro negro. Componer también en escenas
  estrechas. Las bandas de papel se eligen por el menor contraste actual:
  los fundidos de paleta restan brillo y recortan a negro, no multiplican el
  contraste; el umbral anterior admitía emblemas o rechazaba todas las bandas.
- B: los histogramas independientes de 4 bits juntaban papel gris y dibujo
  negro durante el fundido del título. Usar un histograma de colores exactos
  de ambos extremos para un único fondo compartido.
- C: el render nativo precede al VBlank. ModeStart reutilizaba VRAM/paletas
  antes de ModeBlankDisplay, dejando un frame del cuarto liberado. Aplicar
  ModeBlankDisplay al comienzo de ModeStart solo en PC. Se elimina la gracia
  del latch: no conservar mapas posiblemente liberados durante un frame.

## Reproducción y evidencia

- Estado original: v1.8.1 slot1, carga con build/v181/khcom.exe, frame39 con
  siluetas. Original sin modificar; no desactivar el BuildId.
- Fixture `roomtransition`: sala real del castillo, FadeStartOut blanco y
  MapFixLeaveEntranceHall. Antes: frame199 siluetas; después: blanco plano.
  No es una partida completa ni un test de todas las puertas.
- `python port/tools/test_scene_transitions.py --save <copia-normal.sav>`:
  ventana final leída por SDL_RenderReadPixels (KHCOM_OUTPUT_SHOT, misma
  fuente que KHCOM_RECORD); 1536x864 y 1920x1080. Por resolución: 24 frames
  de idioma, 95 de título/cargar y 16 de cambio de sala. Informe y hojas en
  build/transition-review. El mando físico solo se desactiva para pruebas.
- Comparación inicial: título antiguo falla en frames1605..1650 (cada5);
  versión corregida con ambos márgenes idénticos en los95 frames1560..1654.
- EU/US/JP compilan. PPU: 104/107, los mismos tres fallos semánticos históricos
  semantic_hd_hud_cover_bg/cover_obj/window. Ningún fallo de píxeles hardware.
  build/ppu-conformance/20261016_032800_838173/report.json.

## Publicación

Pendiente completar matriz, ZIP/SHA y sustitución de assets de v1.8.2.
El paquete es ROM-free. Mantener una copia de la distribución anterior para
sus estados rápidos antes de actualizar la carpeta local.
Tráiler v4: continuar tras publicar los bugs siguiendo TRAILER.md.
