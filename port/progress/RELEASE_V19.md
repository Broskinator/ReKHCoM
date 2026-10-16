## x1: segunda optimización verificada (16/10/2026)

- Nueva ruta de composición para filas de mundo con fondos opacos, sin OBJ,
  ventanas ni efectos de color aplicables. Mantiene máscaras de prioridad HD/HUD;
  efectos, mensajes acoplados y sprites conservan la ruta completa.
- Oracle independiente ampliado de 108 a **206/206**: mismos casos con mundo
  ancho activado, recogida de prioridades y combinaciones de ventanas/mezcla.
  Informe build/ppu-conformance/20261016_111841_642473/report.json.
- Campo x1, 1920x1080, SDL dummy: primer par PPU 51,34 -> 39,04 ms; repetición
  48,14 -> 39,34 ms (**18,3% menos en el par repetido**). Presentación 21,36 vs
  21,48 ms prácticamente igual; hashes exactos de la imagen antes/después.
  Batalla x1 28,26 -> 27,16 ms en este par; no sumar porcentajes entre pruebas.
  Informes build/zoom-benchmark/v19-opaque-{before,after}[-repeat]/report.json.
  Son tiempos CPU del render de software, no FPS de un GPU real ni promesa 60fps.
- En curso comparación x2/x3/x4/x5/x6. Aún falta matriz EU/US/JP, menús/puertas,
  ultrawide y reconstruir Android con la nueva optimización antes de publicar.
- Android x86: SRAM EU carga, Sora se mueve, menú normal abre, pausa/reanudación
  mantiene PID; solo 2 ticks CPU durante 3s en background. Sin crashes de runtime
  en la sesión corregida. build/android-runtime-smoke.json. ARM físico pendiente.
- Barajas HD: con la SRAM real y entrada natural al juego, ni foto fija ni
  frames 2220-2241 muestran con claridad el solapamiento reportado. Evidencia
  build/transition-review/deck-{normal-save-hd0,normal-save-hd1,submenu-slide-hd}.
  No declarar corregido por esta ausencia de reproducción. Continúa revisión.

## Compilación tras corregir SHN_ABS

ARMv7 EU/US/JP y x86 EU/US/JP compilan; el verificador ELF pasa en los seis.
APK ARM actual: baf793f4ea7a59a0a55c27eb3e63be36415f982a58bfbfce1777cf35e4c22ace.
APK x86 actual: 17e8302522d3ebdae8513090004bb90f541e72583dd0cdcfccaf6693b6780b5d.
Windows EU incremental pasa. Audio: 24/24 bordes PCM y 800.000 integrales sin
error. Fullscreen comprobado en emulador; ROM y SRAM EU cargan sin modificar
los originales del PC. Continúa la prueba de cargar partida, pausa y campo.

## Retoma: Android en ejecución (16/10/2026)

- APK x86 de prueba instalada en emulador oficial API24. Launcher SAF importó
  ROM EU desde Archivos; el juego alcanza selección de idioma y título, y los
  botones táctiles respondieron. No confundir con validación de ARM en móvil.
- Primer arranque reprodujo SIGSEGV en SoundMainBTM: Android rebasa SHN_ABS,
  incluido gNumMusicPlayers=26; el bucle sale de tabla. -Bsymbolic resuelve
  localmente estas referencias; tras corregirlo el título arranca. Verificador
  ELF en build_android.py rechaza futuras relocaciones a símbolos absolutos.
- AVD aislado en D:/ReKHCoM-build/android-avd (C: sin espacio suficiente).
  Scripts/logs locales build/start_android_emulator.py, build/android_ui.py,
  build/android-{first,second}-launch.log y capturas android-title.png.
- Liberados 1.073.755.621 bytes de ZIPs ya extraídos y con hash oficial
  verificado. Manifiesto build/android-archive-cleanup.json; ROM/saves intactos.
- En prueba: fullscreen Android, recreación de iconos tras reinicio del renderer,
  cierre único del archivo SRAM en errores (antes podía cerrarlo dos veces).
- Pendientes PC: Barajas HD, regresión completa, rendimiento x1, cerrar v1.9.
  Pendientes Android: campo/combate, lifecycle/SRAM, ARM real y alpha pública.
  Stable v1.8.3 intacta. No publicar la APK como validada aún.

# v1.9 — trabajo activo, NO publicada (16/10/2026)

## Pedido vigente

Usuario autoriza todos los pendientes para v1.9: optimizar PC en todas sus
variantes, usar x1 de benchmark, conservar corrección visual/audio, resolver
pendiente HD de Barajas. También construir y probar el port Android (backend
ARM, launcher con selector de ROM y controles táctiles), publicar alpha solo
cuando sea jugable. Autoriza SDK/NDK y sus licencias, así como limpiar temporales.
No sustituir la stable v1.8.3 ni su ZIP mientras v1.9 no esté verificada.

## Cambios implementados y primera validación

- `port_ppu.c`: buffers de canvas/oclusiones crecen hasta el tamaño necesario,
  conservando capacidad en los cambios menú/mundo; cero en la parte nueva.
  Realloc al cambiar resolución; no se cambian dimensiones ni escalas visibles.
- `port_main.c`: presentación al tamaño activo, HUD asignado solo al usarse.
- `DrawStraightAffine`: camino pa=256, pc=0, sin mosaico horizontal. Reutiliza
  únicamente las coordenadas X; clave incluye scroll, BG control, recorte,
  geometría y bandas de arena. Lee VRAM y paleta de nuevo en cada scanline.
  Conserva el camino general para rotación, escala o mosaico.
- Benchmark: `python port/tools/benchmark_zoom.py --label <nombre> --frames 480`.
  Subprocesos aislados, SDL dummy, 1080p, x1/x2/x4, campo y batalla, frame479.
  `build/zoom-benchmark/v183-before/report.json` y `v19-first/report.json`:
  las **seis imágenes finales coinciden SHA-256**. No mide GPU real ni una
  partida completa. Registra capacidad privada y working set por separado.
- Repetición x1 para evitar atribuir diferencias de carga/frecuencia a la mejora:
  `v183-repeat/report.json` frente a `v19-repeat/report.json`.
  Campo PPU 30,09 → 29,35 ms (~2,5%, no mejora sustancial aún).
  Batalla PPU 24,05 → 17,87 ms (~25,7% menos tiempo).
  Commit privado pico en campo x1 361,60 → 322,57 MiB; en x4 la primera
  comparación 356,30 → 284,81 MiB. **No afirmar que baja el RSS:** sus picos
  son similares o ligeramente mayores. Los buffers retienen la capacidad máxima
  alcanzada durante esa ejecución; no reducen capacidad al volver a x4.
- Oracle PPU: 104/107, sin fallos nuevos; los tres históricos son
  semantic_hd_hud_cover_bg/cover_obj/window. Todos los píxeles hardware pasan.
  `build/ppu-conformance/20261016_045819_092143/report.json`.

## Pendientes antes de publicar PC

- [ ] Optimizar más campo/x1, midiendo antes/después con carga comparable.
- [ ] Probar explícitamente crecimiento y cambios de resolución/zoom, HUD, estados,
      menús/puertas/cinemáticas, HD on/off y ultrawide; comparar con v1.8.3.
- [ ] Resolver HD sobre submenú de Barajas; revisar los tres fallos semánticos.
- [ ] Comprobar audio, compilar EU/US/JP y actualizar versión a v1.9 solo al cerrar.
- [ ] Empaquetar, revisar SHA/privacidad, publicar stable y preparar copia local.

## Android

Estudio y bloqueos: `port/android/README.md`. Prototipo `preview.html`, iconos
propios `assets/roll.svg` y `stock.svg`. SVG parseados y rasterizados con sharp;
JS del prototipo pasa análisis sintáctico. El visor del navegador bloqueó file://,
por lo que no se verificó interacción táctil real; no eludir esa restricción.
Todavía **no hay APK ni backend ARM**. No anunciarla como disponible.

Usuario aceptó expresamente licencias SDK/NDK e instalación en toolchain.
Instalador local `build/setup_android_tools.py` (sesión shell 98801 al escribir
este checkpoint): descarga SDK commandline-tools latest, NDK r30
30.0.16248370 y Temurin JDK17 desde Google/Adoptium; verifica SHA oficiales.
Instalación en `../toolchain/android/`, manifiesto de URLs/hashes en
`../toolchain/android/downloads.json`. Confirmar finalización antes de utilizar.
Faltan SDK platform/build-tools, SDL Android, Gradle/CMake y proyecto nativo.
La compilación ARMv7 necesita sustituir PE/Win32, GDI, ensamblador COFF y el
manejador x86 de fallos BIOS; arm64 además requiere resolver punteros de 32 bits.

## Limpieza y preservación

Se eliminaron 1.377 capturas `out_*.bmp` / `shot_*.bmp` de ocho carpetas de
review dentro de `build`, todas verificadas bajo el workspace. 6,50 GiB libres
recuperados, unos 12,68 GiB disponibles antes de instalar Android. Manifiesto
`build/cleanup-20261016.json`. Se conservan PNGs de evidencia, ejecutables,
informes, ROMs, partidas/estados, herramientas, tráiler final y sus fuentes.
No tocar los cuatro archivos gráficos no trackeados del usuario en la raíz.

## Integración / continuación

Rama local `claude/windows-5`; subir explícitamente a main y codex/v1.5-fixes.
Última stable: tag v1.8.3, release 414677687, commit juego cfc5b6392,
documentación d462f08cc. Android/diseño/build_all subido en 6d5cdbaca.
No usar `git add .` por los gráficos privados no trackeados.


## Actualización Android y HD (último checkpoint)

- Herramientas instaladas y verificadas. SDK CLI actual usa android.exe sdk
  install por paquete; sdkmanager.bat acepta argumentos viejos pero no instaló
  todas las plataformas. Ya existen platform35/build-tools35.0.0/CMake3.22.1.
- Android CMake pasa; librekhcom_eu.so/us.so/jp.so compilan. SDL3 y núcleos
  ARMv7 ABI21; las secciones khdata/khbss y SDL_main se exportan correctamente.
- Primera APK debug compilada (NO PUBLICADA), manifiesto runtime_tested=false.
  build/releases/ReKHCoM-v1.9-android-alpha1-armeabi-v7a-debug.apk.
  SHA inicial a4e06be306f24e3a49bc744cf7ee3473f416645ca7e6d043ab564159005d3557;
  se deberá reconstruir porque los últimos cambios de lifecycle son posteriores.
- Launcher Java real: SAF ROM con SHA, importar/exportar SRAM, configuración,
  idioma y tres núcleos cargados por separado en :game. Multitoque SDL real,
  iconos embebidos desde SVG; pausa en background implementada sin probar.
- build_android.py y build_all.py --platform android automatizan toda la matriz.
- No hay dispositivo adb conectado. Instalando emulator e imagen API24 x86 en
  sesión 13007; el emulador moderno en host x86 no soporta ARMv7 directamente.
  Próximo paso: permitir ABI x86 (también 32bit) como build de prueba aparte,
  ejecutar allí y distinguir esa evidencia de probar ARM en un móvil.
- Gradle primer intento falló por no definir repositorios de dependencias de
  app (aapt2); corregido dependencyResolutionManagement, siguiente build pasó.
- Error inicial ARM por malloc.h de juego sombreando bionic: -iquote en CMake.
- Oclusión HD HUD corregida: se usa posición OAM original como en RenderHud,
  conservando la excepción del mensaje propio. Oracle ahora **108/108**; agregado
  semantic_hd_hud_message para no volver a ocultar diálogos propios.
  build/ppu-conformance/20261016_101342_964485/report.json.
- Intentos adicionales de filas uniformes / caché de map entries descartados:
  no mostraron mejora medible en campo; no están en el código. Se conserva solo
  la optimización affine y de capacidad verificada en 3fe40fe29.
- Falta revisar HD sobre submenú real de Barajas (estilo0 y BG estilo2), y toda
  la matriz Windows/Android antes de publicar v1.9/alpha. v1.8.3 no modificada.
