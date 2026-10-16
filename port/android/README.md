# Android — estudio del port (16/10/2026)

Estado real: **no existe todavía un juego Android compilable ni una APK jugable**.
Windows v1.8.3 sigue siendo la única estable. Los controles de esta carpeta son
un diseño/prototipo, no una captura del juego ejecutándose en Android.

## Herramientas y motor recomendados

| Pieza | Elección | Motivo |
|---|---|---|
| IDE | Android Studio; también build por línea de comandos | SDK Manager, profiler, emuladores y dispositivos |
| Código del juego | C + Android NDK/Clang + CMake/Ninja | Reutilizar el port y sus correcciones |
| Ventana, audio y mando | SDL3, con SDLActivity | Ya es la capa utilizada en PC |
| Presentación 2D | SDL_Renderer con backend GLES, escalado nearest | Subir el framebuffer y dibujar controles en pocas llamadas |
| Launcher | Activity nativa Java/Kotlin, vistas simples | Selector de ROM, ajustes, partidas, sin un segundo motor |
| Archivos | Storage Access Framework / ACTION_OPEN_DOCUMENT | Abrir desde Archivos y proveedores, sin permisos globales |
| Texto HD | FreeType o SDL_ttf, caché de glifos | Sustituir GDI sin rasterizar cada fotograma |
| Medición | Android Studio Profiler, Perfetto, adb; luego dispositivo modesto | Medir CPU, memoria, audio y temperatura |

No recomiendo migrar a Unity/Godot para esta primera adaptación: no resuelve las
dependencias de memoria del núcleo. El PPU de software existente debe conservar
prioridades, ventanas, mezclas y efectos de la GBA. La GPU presentaría/escalaría
su salida y el overlay; no equivale a trasladar automáticamente todo el PPU a GPU.

SDL documenta SDK 35+, NDK r28c+ y API mínima 21 en su guía actual. Fijar versiones
exactas del SDK/NDK/Gradle/SDL en el proyecto al implementarlo; mínimo propuesto
Android 8/API26 para simplificar la primera alpha, pendiente de validar.

## Bloqueos comprobados en el código

1. `port/include/port/port.h`, README y estructuras del juego requieren punteros
   de 32 bits. `port_memory.c` escribe punteros nativos dentro de palabras ROM de
   32 bits. **No basta con cambiar el compilador a arm64.** ARMv7 puede servir de
   primer hito, pero no cubre teléfonos que solo admiten apps de 64 bits.
2. `port_memory.c` reserva una sección PE hasta 0x0E010000 y depende de direcciones
   GBA absolutas. Android carga bibliotecas ELF/PIE con ASLR: estudiar mapeos
   puntuales sin reemplazar memoria ajena y abortar ante conflictos. No usar
   MAP_FIXED a ciegas. Las regiones pequeñas deben respetar el tamaño de página.
3. `port_biosfault.c` interpreta instrucciones x86 y registros CONTEXT de Windows
   para accesos BIOS/espejos y divisiones por cero. No se puede usar en ARM.
   Registrar y resolver los accesos conocidos mediante helpers explícitos;
   verificar el comportamiento original, en vez de copiar ese manejador.
4. `port_sections.c` y `port_state.c` leen PE/GetModuleHandle. Sustituir por límites
   ELF definidos por linker. Estados rápidos de Windows no son intercambiables;
   preservar la compatibilidad de los `.sav` normales.
5. `gen_pc_assets.py` emite ensamblador COFF con `_symbol` y `.rdata,"dr"`.
   Hace falta salida ELF y relocaciones compatibles con cada ABI.
6. Texto HD usa GDI/windows.h; el launcher usa Win32, OpenGL de escritorio y
   CreateProcess. El launcher Android necesita su propia capa nativa/JNI.
7. Faltan manejo de pausa/reanudación, pérdida de superficie, interrupciones de
   audio, safe areas, multitoque y persistencia al salir al launcher.

En este entorno no se encontraron Java, adb, Gradle/CMake en PATH ni un SDK bajo
LOCALAPPDATA/Android. Instalar herramientas no elimina los bloqueos anteriores.
Por ello no se publica una alpha vacía o un launcher que no pueda ejecutar el juego.

## Flujo del launcher

1. «Cargar ROM» abre ACTION_OPEN_DOCUMENT, CATEGORY_OPENABLE, MIME genérico
   (los proveedores no siempre identifican `.gba`). Validar el contenido real.
2. Leer en streaming hacia un temporal privado, limitar a 32 MiB, calcular SHA-1
   y verificar código/región y los hashes admitidos por PC. Borrar el temporal si
   falla; renombrarlo solo tras completar la validación. No cargar dos copias en RAM.
3. Mostrar Europa/América/Japón e idiomas correspondientes. Guardar `.sav` en
   almacenamiento privado; importar/exportar mediante SAF. No solicitar acceso a
   todos los archivos ni incluir ROMs en APKs.
4. Una actividad SDL en proceso `:game` cargaría solo la biblioteca de la región
   seleccionada. Las tres tienen globals/símbolos coincidentes: no cargarlas juntas.
   Al cambiar de región, guardar y cerrar ese proceso de forma controlada antes
   de iniciar otro; revisar ese ciclo con SDL antes de implementarlo.

## Rendimiento en gama baja: trabajo concreto

- Mantener lógica a 59.7275 Hz. Escalar la presentación, no acelerar el juego.
- El PPU reserva **13 bytes por píxel máximo** (dos buffers RGBA y máscaras
  de 1+1+1+2 bytes), y port_main otros **8 bytes/píxel** para composición/HUD.
  A 2560×1440 son **73.83 MiB**, sin contar otros buffers, ROM, texturas ni audio.
  Es capacidad reservada/calculada, no una medición de RSS o consumo real.
- Para móvil, convertir esas capacidades globales a asignación según el canvas
  activo, crecer con cuidado al cambiar zoom y reutilizar buffers. Por ejemplo,
  el mismo subtotal a 640×360 sería 4.61 MiB. No imponer ese canvas si recorta
  la escena; definir límites coherentes por aspecto y zoom.
- Mapear solo las regiones GBA necesarias: la reserva PE contigua de Windows
  no debe convertirse en una asignación Android enorme.
- Perfil ligero: texto pixelado, sin HD-2D, caché acotada de glifos, márgenes
  simples cuando sea posible, nearest y resolución interna moderada. Exponer
  ajustes sin cambiar partidas ni dificultad.
- Comenzar con el mezclador SDL actual, cola pequeña con protección ante
  underruns. Medir antes de cambiar a Oboe/AAudio; no sumar dos mezcladores.
- Pausar CPU/audio al ir a fondo; descartar todas las pulsaciones activas.
  Estudiar Swappy solo si hace falta integrar frame pacing EGL: no añadirlo
  junto al pacing actual sin definir cuál controla la presentación.
- Medir percentiles de frame time, underruns, PSS, temperatura y batería en
  sesiones de 15–30 min; no prometer «gama baja» basándose en el emulador.

## Controles táctiles

Referencia funcional: distribución GBA landscape de RetroArch (D-pad izquierdo,
A/B diagonales a la derecha, L/R, SELECT/START). Se dibuja arte propio,
sin reutilizar sus imágenes. Ver `assets/` y `preview.html`.

- D-pad con diagonales; A/B y L/R de GBA; START/SELECT independientes.
- Botón de rodar con silueta propia de Sora encogido, pelo en puntas y zapatos;
  flecha curva. Botón de apilar con tres naipes y símbolo de suma.
- Mismo marco blanco, fondo oscuro y transparencia que el resto de botones;
  respuesta visual al pulsar. Opacidad y tamaño ajustables; áreas táctiles
  mayores que el dibujo, mínimo 48dp, dentro de los insets del dispositivo.
- Multitoque por finger ID. Liberar solo la acción del dedo que se levanta;
  cancelar todo al perder foco. Un dedo debe deslizarse entre direcciones.
- Rodar/apilar usarán PORT_KEY_DODGE / PORT_KEY_STOCK del port, sin inventar
  macros temporizadas de A/B/L/R. Mantener el mismo comportamiento que PC.
- Prototipo HTML muestra pulsaciones simultáneas; no controla el juego todavía.
  El layout sobre la imagen de PC solo sirve para revisar tamaño y posición.

## Compilación conjunta y condiciones de alpha

Ya existe `python port/tools/build_all.py`: configura y compila EU/US/JP para
Windows, crea manifiesto de hashes y opcionalmente empaqueta. No publica ni
cambia versiones. `--dry-run` muestra el plan; `--platform android` falla
explícitamente mientras no exista backend Android.

La siguiente etapa es compartir lista de fuentes/opciones con CMake: matriz
`plataforma × región × ABI`, carpeta por combinación, una sola versión de producto,
un APK con bibliotecas regionales (o splits por ABI) y un ZIP Windows con las tres
regiones. Artefactos fuera del código generado. Las ROMs privadas solo en el
paso de generación autorizado; jamás en logs, cachés públicas o artefactos CI.

Una alpha jugable necesita: boot y SHA de ROM, juego/carga/guardado real, batalla
con audio, menús y cinemática, controles simultáneos, suspender/reanudar, pruebas
de instalación en un ABI admitido y dispositivo real. Publicar como prerelease
Android junto a v1.8.3 stable, no reemplazar ni retocar su ZIP. Mientras no pase,
el estado es «port en preparación», no «Android disponible».

## Fuentes primarias consultadas

- [SDL3 Android: Activity, CMake y ciclo de vida](https://wiki.libsdl.org/SDL3/README-android)
- [Android: Storage Access Framework](https://developer.android.com/training/data-storage/shared/documents-files)
- [Android NDK: ABIs](https://developer.android.com/ndk/guides/abis)
- [Android: compatibilidad 64 bits](https://developer.android.com/games/optimize/64-bit)
- [Android: páginas de 16 KiB](https://developer.android.com/guide/practices/page-sizes)
- [Android: Frame Pacing / Swappy](https://developer.android.com/games/sdk/frame-pacing)
- [RetroArch: layout GBA landscape](https://github.com/libretro/common-overlays/blob/master/gamepads/gba/gba.cfg)
- [RetroArch: overlays de entrada](https://docs.libretro.com/development/retroarch/input/overlay/)
