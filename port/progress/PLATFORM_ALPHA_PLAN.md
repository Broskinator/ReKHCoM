# Plataforma y reparto — 17/10/2026

## Regla del usuario

Windows conserva la estable. Android, web, Linux y Steam Deck se publican como
**alpha**, sin prometer compatibilidad ni rendimiento de dispositivos no probados.
La nueva escala relativa de menus no entra en stable. No subir ROMs, saves ni
estados a Git. Conservar historial y trabajo de Claude; no integrar a ciegas.

## En curso: Android, luego Linux / Steam Deck

- Rama `feature/android-save-import`: launcher con destino regional y banco
  explicito (1..5), importacion/exportacion del banco correcto, guardado de la
  seleccion al recrear Activity y bloqueo de selectores simultaneos.
- `.sav` y `.srm` se interpretan por contenido, sin filtro de extension del picker
  SAF. Acepta SRAM de 32 KiB (relleno FF), 64 KiB y 128 KiB solamente cuando las
  dos mitades de 64 KiB son identicas. No recorta contenedores desconocidos.
- Verifica firma, checksum de al menos una cabecera y un bloque de datos.
  Conserva copias redundantes tal cual para la reparacion nativa del juego.
  EU usa VER13; US/JP VER12. US y JP no pueden distinguirse por esta firma:
  el jugador elige la region correcta. No es conversion entre regiones.
- Escritura temporal + sync, copia previa del archivo destino y rename.
  Exporta SRAM de 64 KiB compatible con el port. No importa savestates.
- `python port/tools/test_android_save_import.py` prueba el parser Java real,
  datos sinteticos, corrupcion, version regional y bancos. Se pueden pasar
  rutas locales adicionales para verificar identidad de partidas privadas.
- APK siguiente: `1.9-android-alpha2`, versionCode 19002. Compilar no demuestra
  funcionamiento en ARM fisico. x86 de emulador y ARMv7 son objetivos distintos.

## Pendientes que no deben perderse

1. Android: repetir ejecucion EU/US/JP con APK nueva; probar picker SAF real,
   restauracion de Activity, import/export y multitouch. HD y quickstates siguen
   desactivados. ARM64 requiere adaptar punteros de 32 bits; no esta resuelto.
2. Cheats: submenu, todo OFF por defecto; Riku disponible sin falsear la bandera
   de Sora completado. Cheats conocidos implementados por estructuras/funciones
   del juego, no codigos de direcciones de otra region. Editor de cartas con
   region + banco + ranura explicitos, checksum nativo, backup y limites.
   **Todavia no implementado.**
3. Linux: backend ELF32, SDL3, memoria GBA y fuentes portables. Steam Deck es
   objetivo especifico: SteamOS, mando, 1280x800, modo juego, selector de archivos
   utilizable y persistencia de saves. No anunciar Deck Verified sin certificacion.
   **Sin build Linux ni prueba Deck por ahora.**
4. 4:3 con todos los zooms y texto HD, sin deformar. Conservar tambien el modo
   original GBA 3:2. El usuario pidio conservar forma original; 4:3 es otra opcion.
   Rama `feature/aspect-43-zoom`: opciones 4:3 y GBA 3:2 implementadas,
   geometria 49 combinaciones PASS y build EU Windows correcto. Regresion visual
   en todos los zooms y texto HD aun pendiente. No integrado en main/stable.
5. Web: rama `fix/web-mobile-files-controls`, commit 7956d77f6, subida pero NO
   integrada ni desplegada. Contiene correcciones de teclado/foco/fullscreen,
   import/export y compartir a Archivos. JS y C pasan tests; teclado real de
   Chrome comprobado con fixture sin ROM. Touch real en navegador y dispositivos
   iOS/Android sigue pendiente. Root Pages sigue Web Alpha 2.
6. Menus: rama `feature/relative-menu-scale`, geometria preliminar aun sin integrar
   al render. No cortar framebuffer por tercios: puede partir guantes, badges y
   texto. Anclar grupos completos y fondos propios; probar entrada/salida.
7. Rendimiento x1: baseline `build/zoom-benchmark/relative-menu-baseline/report.json`.
   Field PPU 28.34ms, battle 19.22ms (SDL dummy/software; no FPS de GPU real).
   Optimizar con comparacion de pixels y regresion PPU, sin cache invalida entre
   HBlank. No hay nueva optimizacion x1 en esta rama.

## Licencias y herramientas

Usuario autorizo instalar SDK/NDK y acepto licencias necesarias. AVD local
`D:/ReKHCoM-build/android-avd/ReKHCoMTest.avd`; definir ANDROID_AVD_HOME al lanzarlo.
Usar app separada `com.rekhcom.android.smoke` para pruebas; nunca limpiar la app
del jugador. No publicar APK como probada en telefono por pasar emulador x86.
