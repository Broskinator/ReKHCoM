# Android de 64 bits (21/10/2026)

Pedido del usuario: Android con compatibilidad completa, replicar los errores, batería completa
y arreglarlos. Este archivo es el estado y el plan para retomarlo (otro Claude o Codex).

## Por qué

La app actual es solo de 32 bits (armeabi-v7a; x86 para emuladores) y reserva el espacio de la
GBA en sus direcciones reales (0x02000000-0x0E010000, `port/android/native/port_memory_android.c`).

- #22 y #34 ("Cannot reserve the GBA address space"): en algunos teléfonos (Pixel según un
  reporte, Huawei Mate XT con HarmonyOS 4.2 / Android 12) algo ya ocupa ese rango. Los chips
  nuevos sin AArch32 corren las apps de 32 bits con un traductor binario, y eso explica que pase en
  varias marcas; los Pixel 7 en adelante directamente no ejecutan apps de 32 bits.
- Las imágenes de emulador de Google para PC (x86_64) de Android 13, 14 y 15 ya no traen 32 bits
  (`ro.system.product.cpu.abilist32=` vacío): el fallo no se puede reproducir ahí. Solo la de
  Android 7 (x86) corre la app actual.
- Otro port de GBA (Pokémon Emerald multiplataforma) tiene la misma limitación: Android solo ARM de
  32 bits.

Paliativo ya publicado en main (0b5fce2f4): la librería reserva el rango al cargarse (constructor),
antes que drivers y heap de Java, y el mensaje de error lista qué hay en el rango.

## Cómo

Lo mismo que Switch (`port/switch/README.md`): el juego y la capa del port como imagen AArch64 con
punteros de 32 bits (clang `arm64_32`, Mach-O), con el espacio GBA reubicable (`PORT_GBA_RELOC`, las
listas de punteros de la ROM en `port/data/rom_pointers_*.inc`), y un host de 64 bits que la carga
y le sirve el puente (`port/ps5/shared/bridge_list.h`). Probado en Switch/Vita: título, menús y
combate idénticos píxel a píxel en la VM ARM64.

Diferencias para Android:
1. Dirección de la imagen: en procesos de 64 bits ART usa los 4 GB bajos (heap en 0x12c00000, boot
   image ~0x70000000). La imagen no puede ir en 0x10000000 como en Switch: medir los mapas de
   procesos de 64 bits en los emuladores y elegir una base libre (alta dentro de los 4 GB bajos),
   reservada desde el constructor de la librería.
2. Host: `port/android64/` con SDL3 real (64 bits): gfx con `SDL_RenderGeometry` (los HostVertex
   del puente son triángulos), audio con SDL audio stream, mandos SDL, archivos en el directorio de
   la app, y los controles táctiles (`touch_android.c`) del lado del host, sumando botones al pad.
3. APK: arm64-v8a (teléfonos) + x86_64 solo si hace falta para pruebas (las imágenes x86_64 de
   API 30+ traducen arm64: `abilist64=x86_64,arm64-v8a`).
4. Mantener la de 32 bits como respaldo hasta que la de 64 bits pase la batería.

## Batería

- Escritorio (atrapa lo que cierra Android: `bios:` NULL/BIOS/div0): `debug_kit.py run --build
  --versions eu,us,jp` + `report` (combates, Riku, eventos, 16:9, salas, monkey, escenas).
- Android: `test_android_scenarios.py` en emuladores Android 13/14/15 (x86_64, perfiles Pixel y
  genérico) con el APK de 64 bits.

## Estado

- [x] #33 (cierre antes del combate con Trickmaster): reproducido en PC (EVENT_099, actor NULL en
  task_bos_tm_1 y event_chara_1), arreglado con BIOS_PTR (0b5fce2f4).
- [x] Imágenes Android 13/14/15 x86_64 descargadas (SDK en toolchain/android/sdk).
- [ ] Malla de escritorio completa en EU/US/JP (corriendo 21/10).
- [ ] Mapa de memoria de procesos de 64 bits → base de la imagen.
- [ ] Host Android de 64 bits.
- [ ] Batería en emuladores; responder #22 y #34 con el APK de 64 bits.
