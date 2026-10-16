# Entrega Codex: rendimiento PPU, primer bloque

Base: `a615b59b3`, reparto v1.7 en CODEX_COORDINACION.md. Solo Claude integra,
pushea y publica. Codex deja commits locales en `codex/ppu-performance`.

Se creÃ³ `../khcom-codex-perf` porque `khcom` conserva cambios anteriores sin
commitear (HUD) y estÃ¡ en una base antigua. No se alterÃ³ ese checkout ni los
archivos de interfaz/escalado que estÃ¡ trabajando Claude en `khcom-claude`.

## Commits para integrar

- `8f567fea3`: el harness reserva primero las direcciones GBA y despuÃ©s el heap
  del lienzo; aÃ±ade stub PortFatal. Sin esto no enlazaba y podÃ­a abortar la
  reserva por colisiÃ³n del heap con las direcciones fijas.
- `a31966638`: ComposeLine reutiliza la bÃºsqueda de la primera capa visible
  para composiciÃ³n y oclusiÃ³n HD; busca segunda capa solo si necesita alfa.
  PortHd2dActive y detecciÃ³n de fondos de diÃ¡logo se evalÃºan por lÃ­nea.
  DrawTextBg decodifica las extensiones de mundo por tramos de hasta ocho pÃ­xeles
  del mismo tile, con palette/flip/transparencia conservados. Los glifos UI
  conservan el camino escalar y su orden de registro. La cachÃ© restante es local
  a la lÃ­nea: no sobrevive HBlank, DMA ni cambios de scroll.
- El commit posterior de evidencia inicializa los diagnÃ³sticos del harness y
  contiene este informe. Los campos cap/base no se asignan para texto OBJ;
  el buffer de la prueba debe iniciarse a cero para no comparar basura del stack.

No cambia configuraciÃ³n, HUD semÃ¡ntico, mÃ¡rgenes, cabeceras ni escalas. No aÃ±ade
hilos. No reemplaza ejecutables/partidas del usuario. Solo compilaciÃ³n EU propia.

## VerificaciÃ³n

- Oracle independiente: **104/107**, antes y despuÃ©s. Los mismos tres fallos
  preexistentes: semantic_hd_hud_cover_bg, semantic_hd_hud_cover_obj y
  semantic_hd_hud_window. No se han contado como resueltos.
- ComparaciÃ³n directa de ambos renderers usando el mismo harness actualizado:
  **107/107** fixtures con idÃ©nticos pÃ­xeles y diagnÃ³sticos.
- Extensiones 1920Ã—1080 sintÃ©ticas, 4bpp/8bpp, con/sin mosaico, coordenadas
  negativas, flips y fuera de mapa: cuatro buffers de 8.294.400 bytes idÃ©nticos.
- **42 capturas reales idÃ©nticas**: campo, combate y Estado/retorno a campo a
  1920Ã—1080 (zoom x1) y 5120Ã—1440 (zoom x2), seis frames por caso; mÃ¡s campo
  1920Ã—1080 zoom x2. ROM EU/espaÃ±ol, texto HD activado, arte HUD experimental
  desactivado. Las capturas de la primera pasada de campo x1 preceden al Ãºltimo
  ajuste de su bucle de paleta; el caso final se repitió y sus seis capturas
  también son idénticas (build/perf-comparison/final).
- Campo: room:10901, fieldfreeze. Combate: KHCOM_BATTLE=0:1:0 y A en frame60.
  Estado: START180, DOWN280/300/320/340, A420, B640. El log confirma entrada
  mode_status en423 y retorno Mode_MapFld en676. No se usÃ³ entrada directa de modo.
- Capturas en frames180,380,600,645,655,680; salida en721.

Los archivos locales ignorados estÃ¡n en build/perf-comparison, build/wide-differential
y build/ppu-conformance. compare_perf.py, compare_wide.py y compare_fixture_all.py
son runners locales en build; las nuevas fixtures reutilizables quedan en
port/tools/ppu_conformance.c. Para seleccionar un renderer anterior al compilar
el harness, definir PPU_SOURCE a la ruta absoluta de una copia de port_ppu.c.
Las fixtures wide_world_4bpp, wide_world_8bpp y sus variantes _mosaic aceptan
argumentos `nombre salida.bin [repeticiones]`; el Ãºltimo imprime ms/frame.

## Tiempos medidos

KHCOM_PROFILE=1, software renderer SDL dummy, KHCOM_FAST=1; medias de bloques de
240 frames. Comparar PPU, no present. Hubo otras pruebas/procesos durante parte
de las mediciones: sirven para localizar la ganancia, no como benchmark exclusivo.
Se omite el primer bloque, que incluye el arranque y la entrada de la escena.

| Escena / lienzo interno | PPU antes, ms | PPU despuÃ©s, ms |
|---|---:|---:|
| Campo 1920Ã—1080 x1 | 131,95 / 131,54 | 81,67 / 78,14 (primera pasada) |
| Combate 1920Ã—1080 x1 | 70,88 / 67,41 | 51,48 / 56,34 |
| Estado/retorno 1920Ã—1080 | 89,99 / 27,33 | 44,83 / 15,36 |
| Campo ultrawide, 2560Ã—720 | 150,06 / 167,61 | 63,97 / 64,94 |
| Combate ultrawide, 2560Ã—720 | 58,47 / 49,36 | 31,05 / 29,33 |
| Estado/retorno ultrawide | 130,73 / 29,22 | 38,54 / 13,79 |
| Campo 1920Ã—1080 x2, 960Ã—540 | 32,34 / 30,72 | 11,80 / 12,68 |

La PPU de campo x2 queda por debajo de 16,74 ms en esta prueba. **El objetivo de
60 fps con lienzo x1 completo sigue pendiente.** PresentaciÃ³n software tambiÃ©n
puede superar ese presupuesto y no se ha modificado en este bloque.

## Continuar

1. Claude integra los commits locales y comprueba sobre su interfaz nueva.
2. Repetir perfil con renderizador real y sin procesos de pruebas concurrentes;
   verificar las restantes familias de menÃºs y regiones.
3. Para otra mejora grande, perfilar primero DrawTextBg y la composiciÃ³n. No
   lanzar RenderLine en hilos directamente: hay buffers de lÃ­nea compartidos,
   estado afÃ­n/mosaico, colecciÃ³n de glifos y HBlank/DMA que requieren aislamiento
   y orden exacto. Mantener comparaciÃ³n binaria con la referencia.
