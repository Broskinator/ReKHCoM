## Web Alpha 2: cierre F11 y artefactos finales (16/10/2026)

- F11 ahora pide fullscreen del canvas (igual Alt+Enter), previene repetición y
  deja Escape al navegador. No depende solo de ocultar barras de Chrome. Test
  de producción JS también prueba la petición F11 y el callback de resolución.
- Código final 292080d89; Pages ac7dd3c372160f04fb2e48d77eba79c54d935468.
  Nueva verificación pública 17/17 hashes PASS. WEB_PUBLICACION.json actualizado.
- Tag web-alpha2, creado en esta tarea, se actualizó con lease desde ca86d0016 a
  292080d89 para incorporar F11. No se cambiaron tags anteriores ni Windows.
- ZIP final 4.386.176 bytes, SHA256
  0134d45a734e51f8237ed4ea8c851a72d4257aff28d7022749951e8deb3358fc.
  Assets reemplazados en el mismo prerelease. Supersede hashes de Alpha2 previos.
- El usuario debe exportar su partida y recargar una pestaña vieja para ejecutar
  el motor nuevo. No intentar cambiar un Wasm activo durante una partida.
- Conserva pendientes de gameplay real regional, F11/Escape manual, persistencia
  real y dispositivos físicos. No declarar todos los bugs universalmente cerrados.

## Web Alpha 2 publicada y verificada (16/10/2026)

- Código ca86d0016; Pages f9827a51ff92f6e8839103b3655dc700a2a44f27.
- https://matymeatboy.github.io/ReKHCom/ . 17/17 archivos públicos coinciden en
  SHA256 y tamaño, motores application/wasm, fuentes incluidas. WEB_PUBLICACION.json.
- Prerelease https://github.com/MatyMeatBoy/ReKHCom/releases/tag/web-alpha2 .
  ZIP 4.386.139 bytes, SHA256 1aafa9ab0552df868217a36749d1f3fa48237f80aa6f24a5269cf2f20777ee2f.
  GitHub confirma digest y prerelease=true. La latest estable sigue v1.8.3.
- Portada real Chrome: English por defecto, cambio Español y vuelta English
  verificados. docs/screenshots/web-player.jpg actualizado. Interfaz y lenguaje
  del cartucho son independientes. Alpha2 identifica la web en repo/README/About.
- Se publicaron fix resize/fullscreen, táctil centrado, HD fuentes originales,
  persistencia serializada/banco activo/reset protegido y caché por hash.
- No marcar todos los bugs o plataformas como resueltos: comprobar partida real
  Alpha2 y F11/Escape manual, guardado/bancos, US/JP, móviles y sesiones largas.
  La prueba real de fuente/SDL fue sin ROM; la prueba F11 automatizada fue de
  callbacks controlados porque requestFullscreen no se concedió al clic de tool.
  WEB_ALPHA2_TESTS.json y WEB_PENDIENTES.md conservan evidencia y pendientes.

## Web Alpha 2 lista para publicar (16/10/2026)

- EU/US/JP compilados sin avisos de firma. PPU 206/206; PC EU/US/JP compilan.
- Fuentes reales KHMenu/ComicHearts dibujadas en SDL/WebGL con prueba sin ROM:
  inglés, signos/acentos españoles y subtítulos. No confundir con partida real.
- Native SDL resize verificado 960x540 -> 769x433 -> 960x540. Fullscreen API no
  concedida al clic automatizado del navegador; casos F11/Escape/fullscreenchange
  probados en JS de producción con frontera nativa controlada, no como prueba
  manual de gameplay. El usuario debe confirmar ese ciclo en su partida.
- Pruebas Node de frontend PASS, táctil real C en 5 proporciones PASS,
  controles Android existentes PASS, audio 24/24 + 800k integrales PASS.
- WEB_ALPHA2_TESTS.json contiene el alcance. Falta despliegue/hash/release y
  actualizar evidencia pública. No afirmar que todos los navegadores/dispositivos
  o partidas largas estén verificados. WEB_PENDIENTES.md mantiene lo restante.

## Web alpha2 en desarrollo (16/10/2026)

- Usuario pidió: corregir F11/Escape y resolución, centrar controles táctiles,
  portar HD con fuentes de PC, idiomas web con inglés principal y release alpha.
- Implementado, aún en verificación: resize coalescido por ResizeObserver,
  fullscreenchange y window/visualViewport; export C encola tamaño, SDL aplica
  entre frames. Guardado serializado, banco activo al exportar, reset espera
  sync en Asyncify y no reinicia si falla. Banner persistente ante fallo IDBFS.
- HD Web ahora rasteriza con Canvas2D las mismas KHMenu/ComicHearts. La colocación
  se extrajo sin cambios a port_hdtext_layout.inc compartido con Windows.
  Caché 2 escalas/estilo, 1024 JP por font; sin duplicar imágenes JS. Falta prueba
  visual y regional. EXP sigue filtrado en PPU. Fallo de fuente vuelve a SD.
- UI web English por defecto, selector English/Español separado de idioma ROM.
- Táctil Web usa plano 16:9 centrado con margen; dibujo y hit tests usan la misma
  transformación. Android sin cambios de geometría.
- test_web_runtime.cjs PASS: tamaños fullscreen/Escape/F11, resize coalescido,
  idiomas, nombres por banco, sync serializado y fallo quota.
  web_touch_conformance.c PASS: centro y pulsaciones en 5 proporciones.
- Compilando EU/US/JP: build/web-alpha2-build.log (sesión75596). Emsdk6.0.12,
  SDL estático en D:/ReKHCoM-build/web-sdl. No desplegar hasta compilar/probar.
- Próximo: prueba real de fuentes, regresión PC tras extraer layout, caché por
  hash de shell/motores/fuentes; publicar Pages y prerelease web-alpha2.

## GitHub Pages verificado (16/10/2026)

- URL pública operativa: https://matymeatboy.github.io/ReKHCom/ .
- Pages commit 4a7495e45451bf0c3ef8aa9d62c4d7e6ca65dd23, source 5c39af98b.
  Build GitHub: built. Los 12 archivos públicos coinciden por SHA256/tamaño;
  los tres Wasm se sirven como application/wasm. WEB_PUBLICACION.json.
- README/About enlazan Web; captura pública docs/screenshots/web-player.jpg.
  La portada conserva créditos, requisitos de ROM y foco en funciones del player.
- Prueba de gameplay sigue siendo EU local realizada por el usuario; la captura
  pública es del launcher. No confundir integridad/publicación con partida larga.
- Pendientes completos en WEB_PENDIENTES.md. HD Web NO está implementado; SD
  forzado evita diálogos vacíos. Próximo: guardar/recargar/exportar, US/JP reales,
  audio/pausa y móviles antes de asumir compatibilidad completa.

## Web publicado en rama y limpieza final (16/10/2026)

- Source d7f16c549 pusheado main/codex/v1.5-fixes. gh-pages e57d1e0ae con
  EU/US/JP finales, guardia SD por frame y manifiesto. Pages construyendo:
  https://matymeatboy.github.io/ReKHCom/ . About y README enlazan el juego.
- publish_web.py conserva bytes exactos con hash-object --no-filters para que
  autocrlf no invalide hashes en Windows. Regenerar manifiesto tras source commit.
- Pruebas finales: ABI Wasm PASS; PPU PC 206/206; PC EU/US/JP compilan;
  táctil PASS; stock EU/US/JP PASS; audio 24/24 + 800.000 integrales sin error.
- Limpieza: 134 duplicados de pruebas eliminados (1.155.677.212 bytes), 20
  temporales Windows >7 días (7.142.395 bytes), BMP generados convertidos a PNG
  con igualdad RGBA verificada (4.562.465.029 bytes ahorrados). Total 5.725.284.636
  bytes (~5,73 GB). C: ~9,7 GB libres. Inventario hasta profundidad 5 de los otros
  proyectos no encontró .tmp/.temp/.dmp antiguos; no se borró nada ambiguo.
- Manifiestos privados build/cleanup-20261016-{web-review,windows-temp,lossless-images}.json.
  Se conservaron fuentes, SDK, releases, ROMs/SRAM/states, trailer y cuatro imágenes
  del usuario en raíz. No limpiar toolchains activos para ganar espacio.

## Navegador: alpha y seguimiento (16/10/2026)

- Prioridad del usuario: publicar el juego real en GitHub Pages. EU probado por
  usuario en Chrome hasta diálogos iniciales; SD correcto. No afirmar test de
  combate Web, sesiones largas ni rendimiento móvil todavía.
- Emsdk 6.0.12 en D:/ReKHCoM-build/toolchain/emsdk; SDL3 3.4.18 estático en
  D:/ReKHCoM-build/web-sdl. build_all.py --platform web compila EU/US/JP.
- build_web.py transforma imports fijos LLVM a constantes; conserva ABI wasm32,
  relocaciones de funciones reales y Asyncify para bucles de juego/películas.
- test_web_abi.py PASS: constantes, hardware fijo, globales nativos, espera
  Asyncify y callback de retorno convertido. build/web-abi/report.json.
- Se corrigieron prototipos faltantes (Wasm atrapaba firmas void/int) y el export
  HEAPU8 del loader. La pantalla vacía al elegir HD era configuración activada
  después del arranque por Re:Chain: Web fuerza SD por frame, como Android.
- Port web: ROM local SHA-1, SRAM/config IndexedDB, importar/exportar .sav,
  controles teclado/mando/táctiles opcionales, pausa hidden y canvas sin DPR.
  Persistencia y mobile están implementados pero pendientes de pruebas completas.
- PC EU/US/JP recompilado; oracle PPU 206/206 PASS:
  build/ppu-conformance/20261016_130024_063894/report.json.
- Lista específica y prioridades futuras: [WEB_PENDIENTES.md](WEB_PENDIENTES.md).
  Consultarla antes de retomar HD, guardados, móviles y rendimiento.
- Publicación en preparación con publish_web.py: allowlist de archivos, hashes y
  árbol gh-pages aislado. No ROMs/SRAM/config. La estable PC sigue v1.8.3.
- Limpieza autorizada: 1.155.677.212 bytes de duplicados de transition-review;
  7.142.395 bytes de temporales Windows >7 días. Logs build/cleanup-20261016-*.
  Capturas BMP antiguas se convierten a PNG con verificación píxel a píxel;
  conservar fuentes, releases, ROMs, estados y partidas.

## Android: combate regional y nuevo objetivo Web (16/10/2026)

- Batalla Android EU/US/JP: los tres llegan a frame600, producen imagen real
  960x540 y salen en frame900 sin crash tras la corrección de stock. Informe
  build/android-runtime-battles/report.json; capturas eu/us/jp.png.
- US fallaba en IsKindPairThenNonSummonOfCategory (NULL+0x48, stockCount<3).
  Las dos funciones IsKindPairThen{NonSummonOfCategory,Summon} ahora usan también
  en PLATFORM_PC la comprobación count==3 que ya tenía EU. Se aplica a Android
  y Windows, conserva combos válidos y evita depender de SEH en US/JP.
  test_stock_count.py pasa EU/US/JP con stocks incompletos y combos válidos.
- Bibliotecas Android exportan solo SDL_main (exports.map). Dynsym EU x86
  75.383 -> 168 entradas; las restantes son imports. Globales/secciones siguen
  resolviéndose internamente y el juego arranca. Símbolos de debug conservados
  en build/android-native*, los staged .so usan --strip-unneeded.
- build_android.py --smoke crea otra app com.rekhcom.android.smoke y artefacto
  marcado test_only, con batalla/captura/salida automáticas via SDL_ENV. No subir
  ese APK. El APK normal no contiene SDL_ENV. test_android_runtime.py exige
  serial explícito y usa solo el almacenamiento de la app de pruebas.
- Los binarios empaquetados se cotejan por SHA con los staged, y Gradle clean
  evita huecos de empaquetado incremental. NDK fijado también en Gradle.
- APK ARM actual (tras stock/export/strip/launcher) compilada, NO publicada:
  SHA256 898c19a5bd862d27844ff6d0d80a49f7ceaa7aa5d91afbdd23dc694850260a0f.
  ARM físico no probado. x86 smoke probado; normal debe reconstruirse también.
- Usuario aclaró HD: Ver barajas, pantalla de añadir/quitar naipes. Capturando
  ahora AddGrid / AddValueSelect, no solo el popup de comandos.
- NUEVO pedido autorizado: port navegador con ROM desde Archivos, hospedado
  en GitHub Pages. Comenzó análisis SDL3/Emscripten/Asyncify y memoria GBA.
  Pages aún no existe (API404); no publicar launcher vacío como port jugable.
  Resolver ABI/símbolos de Wasm y bucle antes de desplegar; misma prioridad de
  no regresiones PC/Android. Documentar en reparto a cada avance.

## Regresión regional y opciones táctiles (16/10/2026)

- EU/US/JP: 18/18 imágenes SDL idénticas a los EXEs publicados en v1.8.3:
  campo, combate y Barajas en 1920x1080 y 2560x720. Informe
  build/v19-regions-report.json; test_v19_regions.py extrae solo EXEs de referencia.
- Comparación de composición: 60/60 frames exactos, menús Deck/Mapinspect/Estado
  y puertas, 16:9, 32:9 y resize en caliente. build/v19-composition-regression.json.
- Escalas x2/x3/x4/x5/x6: todos los hashes de imagen coinciden; x1 ya repetido.
- Encontrado/corregido Android: el botón … mandaba bits GBA pero nunca abría
  PortUi (solo escuchaba eventos SDL). El touch ahora abre esa UI y navega por
  la misma ruta teclado; pruebas de multitoque/cancel/foco/diagonales/UI pasan.
  También comprobado en emulador: abre opciones, cambia zoom x3 -> x2, B cierra,
  al reabrir conserva x2 y config zoom=4. Capturas build/android-options-{fixed,
  nav,reopen}.png. No confundir estos controles con el prototipo HTML.
- Launcher Android ofrece solo zooms que caben en la pantalla apaisada; x4 sigue
  por defecto donde cabe. En 540p máximo x3 (160 líneas mínimas del juego), antes
  el selector ofrecía x4 aunque el motor terminaba limitándolo silenciosamente.
- Reconstruir APK ARM y x86 tras este checkpoint (launcher/ruta UI cambiados).
  Alpha y v1.9 todavía sin publicar; v1.8.3 permanece estable. Falta comprobar
  combate Android y todo lo indicado en la cabecera anterior.

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

# Trabajo activo — Android y rendimiento PC v1.9 (16/10/2026)

- [x] v1.8.3 única stable publicada, tags previos conservados, README/capturas actuales.
- [x] Estudio Android, bloqueos nativos y ruta SDL/NDK/launcher SAF: ../android/README.md.
- [x] Diseño táctil GBA, iconos vectoriales propios de rodar/apilar y prototipo HTML.
- [x] build_all.py configura/compila EU/US/JP juntos; compilación incremental verificada.
- [x] Limpieza autorizada: 1.377 BMP generados, 6,50 GiB recuperados. PNG de evidencia,
  informes, ROMs, partidas/estados, ejecutables y tráiler final conservados.
- [x] Android: SDK/NDK/JDK instalados; backend ARMv7 EU/US/JP compila y APK debug generada.
- [~] Android: pruebas de ejecución, lifecycle y controles; APK aún NO publicada.
- [x] Benchmark x1/x2/x4 y primera optimización PC: seis imágenes idénticas.
- [x] Buffers por capacidad activa y caché de coordenadas affine; oracle sin regresiones.
- [~] PC v1.9: campo/x1, HD en submenú de Barajas, matriz de regresión y release.
- [x] Tres fallos semánticos HUD corregidos; oracle ampliado 108/108, mensaje propio visible.

Checkpoint detallado y comandos: RELEASE_V19.md. Batalla x1 repetida: PPU
24,05 → 17,87 ms; campo 30,09 → 29,35 ms. No prometer aún 60fps ni mejora total.

El overlay táctil nativo está conectado; falta validar campo, combate y lifecycle. No anunciar Android disponible.
Detalle limpieza: build/cleanup-20261016.json. No se borraron archivos del usuario.

---

# Relevo activo — v1.8.3 (16/10/2026)

Pedido del usuario a Codex: resolver el guante estirado de Naipes mapa.
Claude dejó sin commitear la extensión de PortMenuBackgroundOnly a
Mapinspect y Deck; Codex verifica e integra ese cambio.

- [x] Reproducir la franja blanca en la ventana final, frame219 a 1536x864.
- [x] Usar los fondos de ambos paneles; conservar el guante original central.
- [x] Antes/después: entrada/salida de Naipes mapa y Ver barajas, tres
  resoluciones y zoom x3/x4/x6; regresión de Estado.
- [x] Compilación EU/US/JP.
- [x] ZIP verificado; copia local v1.8.3 con partidas normales conservadas.
- [x] v1.8.3 publicada como única stable; release v1.8.2 retirada, tag conservado.
- [x] README actualizado para jugadores; 23 capturas de v1.8.3 revisadas.

El texto HD por encima del submenú de Ver barajas sigue siendo otro pendiente.

---

# Relevo activo — v1.8.2 (Codex, 16/10/2026)

El usuario autoriza a Codex a integrar y republicar tras agotarse Claude.
Este bloque sustituye las exclusividades históricas de abajo.

- [x] A: fondo actual en transiciones e idioma sin emblemas repetidos.
- [x] B: histograma exacto compartido; margen derecho del título corregido.
- [x] C: blank antes de reutilizar gráficos; fixture real antes/después.
- [x] Matriz: 270 frames presentados en 1536x864/1920x1080; EU/US/JP compilan.
- [x] v1.8.2 republicada: ZIP/SHA verificados, tag anterior conservado y copia local anterior.
- [x] Tráiler v4 local terminado y verificado; scripts/continuidad en TRAILER_V4_ENTREGA.md.
- [ ] Futuro: HD sobre el submenú de barajas, aplazado en el último reparto.

Detalles: RELEASE_V182.md. Los intentos previos de A-C quedan sustituidos.

---

# Reparto vigente — v1.8.1 (16/10/2026)

- [x] Estado: usar fondos para márgenes de entrada/salida, sin extender avisos NUEVO.
- [x] Derivar el comportamiento del modo actual, sin una bandera que se pierda al cargar estados.
- [x] Quitar pasos intermedios; conservar x4 predeterminado e indicaciones x6/x3.
- [x] Migración de selecciones intermedias antiguas y 41 casos de zoom.
- [~] Comparación visual entrada/salida, avisos activados/desactivados, 16:9/32:9.
- [ ] Publicar v1.8.1 y preparar acceso local.

---

# Reparto vigente — v1.8 (16/10/2026)

Codex integra y publica por pedido del usuario. Claude sin créditos.

- [x] Ocultar «Solo escalas enteras» y «Escalado» en juego y launcher.
- [x] Mantener todos los pasos; anotar x6 como íntegra x1 con aviso y x3 como íntegra x2.
- [x] x4 actual predeterminado, también en ultrawide; migrar sin perder zoom explícito.
- [x] EXP: números originales siempre; quitar HD solo de esos avisos. Posición original conservada.
- [x] Reproducir ranuras 1/2/3 de v1.7.1 sin alterar los originales.
- [x] Relleno de mapa exterior en ranuras 1/3: imagen original idéntica, 16:9 y 32:9.
- [x] Mensajes sin BG: márgenes según fondo renderizado y sus fades.
- [x] Matriz de 32 configuraciones y 3 avisos EXP; HD activado/desactivado idéntico, EU/US/JP y estados nuevos pasan.
- [x] v1.8 pública, assets/digests y latest verificados; carpeta y acceso local preparados.

---

# Reparto vigente — v1.7.1 (16/10/2026)

Codex asume integración y publicación por pedido del usuario; Claude sin créditos.
Referencia actual: CODEX_COORDINACION.md. Los bloques anteriores son históricos.

- [x] Integrar rendimiento Codex sobre interfaz y escalado v1.7 de Claude.
- [x] Capturar ranura 1 de v1.6.7 en un directorio aislado.
- [x] Corregir interpolación PCM fuera de muestra: 24 regresiones pasan.
- [x] Optimizar síntesis PSG: 800.000 integrales idénticas a la referencia.
- [x] Preparar buffer de audio antes de reproducir y al cargar un estado.
- [x] Reducir espera activa; descartar omisión de copia tras detectar regresión ultrawide.
- [x] Recuperar numeración de píxeles v1.4, predeterminado x4 a 1080p y filtro real de enteras/intermedias.
- [x] Verificación regional, estados y capturas contra paquete público v1.7; seis casos Auto y 116 pruebas de zoom.
- [x] Publicada v1.7.1 unificada; ZIP/SHA-256 verificados y acceso local preparado.
- [x] Retiradas 14 releases del lote solicitado; todos los tags remotos intactos.

---

# Entrega v1.5 — reparto vigente (15/10/2026)

El usuario pide dejar continuidad para Claude y pushear cada bloque. Esta sección
reemplaza los repartos históricos de abajo. Rama local `claude/windows-5`, remoto
`origin/codex/v1.5-fixes`. No trabajar simultáneamente sobre render/menús.

**Leer primero [CONTINUIDAD_V15_HUD.md](CONTINUIDAD_V15_HUD.md).**
Codex entrega el bloque actual; Claude puede asumir los pendientes siguientes.
No se ha publicado v1.5 ni retirado v1.4. Guardados del usuario intactos.

- [x] Zoom: normal x4 a 1920×1080; pasos efectivos distintos, test_zoom.py en seis resoluciones. Push deaabb7.
- [x] Arena: continuar textura inferior antes de componer sprites; Ciudad de Paso, Agrabah y País de las Maravillas revisados. Push b0359b4.
- [~] Minimapa: anclaje de sprites a esquina implementado; queda matriz visual de todos los zooms/HUD. Incluido en deaabb7.
- [~] Síntesis: BG1 declarado UI para evitar panel perdido y bloques cian; captura de puerta real a 1920×1080 revisada, panel recuperado.
- [~] Selector: reaplicar desplazamiento de 8 líneas tras tres redibujados; guardia de división por cero en barra de una página. Falta revisión específica tras tutorial de puerta de evento.
- [~] Puerta: eliminado recorte rectangular del sombreado; entrada/salida ordinaria capturada. Validar todos los encuadres y suavidad del haz.
- [~] Submenús: escala entera independiente x5 a 1080p / x6 a 1440p. **Cabeceras/pies desacoplados pendientes**; se retiró el intento defectuoso que cortaba la imagen en franjas.
- [ ] Claude: corregir animación de salida de fondos/márgenes; no declarada terminada.
- [ ] Claude: matriz visual 16:9, 21:9 y 32:9, especialmente 5120×1440; todas las escalas válidas; EU/US/JP.
- [ ] Claude: resolver/atribuir tres fallos del test PPU de oclusión HUD, ver guía.
- [ ] Claude: changelog/README, ZIP sin ROM, acceso local v1.5, publicar y retirar v1.4 tras validar sustituta.
- [ ] Claude: tráiler nuevo con material de la versión terminada. No reutilizar capturas antiguas como evidencia.

Las compilaciones regionales y capturas se detallan en la guía. Los estados `[~]`
son parciales y no equivalen a aceptación visual del usuario.

---

# Reparto de pendientes â€” reordenado el 14 de octubre de 2026

Responsabilidades acordadas con el usuario para trabajar sin solaparse,
ordenadas por las barras de `status.json` que faltan para el 100 %.
`[x]` hecho y verificado, `[~]` hecho sin verificar del todo, `[ ]` pendiente.

## CoordinaciÃ³n (14/10, 04:05)

- 14/10: Codex sin tokens. Mientras no vuelva a trabajar, Claude asume tambiÃ©n
  sus pendientes (widescreen field/battles/menus & events, nÃºmeros HD y su
  revisiÃ³n visual). Cuando Codex retome, se reparte de nuevo aquÃ­.

- Claude es el integrador principal: integra los commits de ambos en
  feat/native-movies, compila EU/US/JP antes de subir y prepara las releases.
  Solo se sube lo que compila en las tres regiones.
- NÃºmeros HD: Codex los cubre todos con el estilo 2 (tiles de BG en barajas y
  sprites de EXP); Claude no duplica. Pendiente: revisiÃ³n visual de colores.
- Herramientas de verificaciÃ³n: Codex tiene test_gameplay_*, test_ppu_conformance
  y test_wide_world_* sin subir; Claude ya subiÃ³ test_rooms, test_submenus,
  test_ppu, test_options y test_songs. Al integrar se unifican (sin borrar
  ninguna) y se anota aquÃ­ cuÃ¡l queda como referencia de cada barra.
- Ãšltimo: partida completa / TAS. DespuÃ©s del 100 %, la funciÃ³n experimental
  con las QoL de la versiÃ³n de PS2 (Re:Chain of Memories) en la de GBA.

## Reparto actual (14/10, tarde) â€” todas las barras al 100 %

CoordinaciÃ³n vigente por peticiÃ³n del usuario: Codex trabaja en el HUD HD y
deja commits locales con evidencia; solo Claude integra, pushea y actualiza
las releases. Prioridad del HUD: preliminar completo antes del ajuste fino.

| QuiÃ©n | Trabajo |
|---|---|
| Claude, junto con el usuario | Las 17 ventanas con mÃ¡rgenes laterales, una por una con su revisiÃ³n (lista abajo). Base: las versiones de la maÃ±ana del 14/10; solo mejoras puntuales. |
| Codex | Seguir mejorando el modo Re:CoM y sus QoL (port_ps2qol.c, la ventana de nueva partida en port_ui.c, kh_com_recom.ini). Regla: con el modo apagado o en la versiÃ³n Original nada cambia respecto al cartucho. |

### Las 17 ventanas (Claude + usuario, en orden)

Avance Codex (14/10): controles QoL y texto rÃ¡pido guardados por separado
por partida/banco en `kh_com_recom.ini`, compatible con registros anteriores.
La dificultad conserva daÃ±o cero y valores negativos. VerificaciÃ³n:
`python port/tools/test_recom.py` (80 combinaciones, cinco bancos, reinicio,
registros antiguos y aislamiento de Original). Sin cambios en los mÃ¡rgenes.

Nuevo experimental a cargo de Codex: HUD HD redibujado con IA y alfa real.
Formato preparado: atlas PNG RGBA + JSON, validaciÃ³n y vistas claro/oscuro
con `port/tools/pack_hd_hud.py`. Piloto integrado del panel HP enemigo con
acabado suavizado, PNG cacheado y opciÃ³n `hd_hud` en ambos menÃºs. Usuario
autoriza placeholders. Ver `HUD_HD_EXPERIMENTAL.md`; pruebas aisladas con
`test_hd_hud.py` superadas en EU/US/JP: recurso ausente y vista Original
idÃ©nticos a la referencia; barra/letras verdes conservan pÃ­xeles y posiciones.
El marco toma lÃ­mites nativos por frame, siguiendo la precisiÃ³n solicitada
por el usuario. No activar piezas con fondo residual o halos.

Base actual sincronizada: prerelease v0.8 y commits de mÃ¡rgenes/README del
14/10. Codex continÃºa encima: extracciÃ³n local de referencias HUD (25 piezas
en combate bÃ¡sico), dÃ­gitos de naipes con contornos CFF de KHMenu y oclusiÃ³n
por sprite. Barras propias en `status.json` â†’ `experiments`, sin modificar
el 100 % del port base. El panel enemigo preliminar ya incluye letras
HP/VT/LP/PV transparentes con relieve y relleno suavizado que conserva el
recorte nativo. PV tiene la esquina superior derecha de la P cerrada en
diagonal. Pruebas aisladas superadas en EU (es/de/it y Riku), US y JP;
pendientes: retratos, panel del jugador y restantes familias del inventario.

1. Diario â€” revisada (14/10)
2. Ver barajas / AÃ±adir-Quitar â€” revisada (14/10)
3. Estado â€” revisada (14/10)
4. Naipes mapa â€” revisada (14/10)
5. Tienda moguri â€” revisada (14/10)
6. Naipes mundo (quitar el tramado) â€” revisada (14/10)
7. Mapamundi â€” revisada (14/10)
8. Elige un mundo â€” revisada (14/10)
9. VÃ³rtice â€” revisada (14/10)
10. Bonus Ã©lite (selecciÃ³n de cartas) â€” revisada (14/10)
11. MenÃº START â€” revisada (14/10)
12. SÃ­ntesis de estancias (puerta y carta) â€” revisada (14/10)
13. Guardar â€” revisada (14/10)
14. Subida de nivel â€” revisada (14/10)
15. SelecciÃ³n de idioma â€” revisada (14/10)
16. Copyright y tÃ­tulo â€” revisada (14/10)
17. PelÃ­culas y entrada al combate â€” revisada (14/10)

## Barras pendientes y dueÃ±o (actualizado 14/10, 08:40 â€” total 98,9 %)

| Barra | % | Peso | DueÃ±o | QuÃ© falta para el 100 % |
|---|---|---|---|---|
| Gameplay verification | 100 | 10 | Claude | Cerrada por decisiÃ³n del usuario: las partidas completas quedan al feedback de los jugadores |

Cerradas hoy por Claude (con evidencia en port/progress/evidence/):
- Widescreen: battles 100 % â€” arenas US/JP 19/19, salida de combate con el
  fixture `battlewin` (combate de depuraciÃ³n y de campo), entrada con el
  corazÃ³n en su proporciÃ³n (battles_wide.txt).
- Widescreen: field maps 100 % â€” todas las salas de historia en EU/US/JP
  (118/120 salas Ãºnicas cada una; las otras dos son un encuentro antes de la captura,
  revisadas aparte), vestÃ­bulos y Bosque de los 100 Acres (fieldmaps_wide.txt).

Ya al 100 %: build, assets, memoria, BIOS, DMA, interrupciones, entrada, m4a,
PSG, PCM, cÃ³dec de bloques, pelÃ­culas, SRAM, reset, widescreen renderer,
widescreen field maps, widescreen battles, widescreen menus & events, estados
guardados, launcher, saltar
intro, versiones USA y japonesa, menÃº de opciones, PPU, texto HD.

## Acabado panorÃ¡mico (era de Codex; lo lleva Claude mientras Codex no vuelva)

- [x] Cristal del castillo: doce ciclos del menÃº conservan una sola tarea.
- [x] Cian en selecciÃ³n de carta de puerta; mÃ¡rgenes negros.
- [x] Entrada a combate: fondo difuminado, animaciÃ³n central en su proporciÃ³n.
- [x] Arenas como cinta de tiles: bandas de sus propios tiles elegidas por
  arena (btl_map.c), sin duplicar jefes ni motivos Ãºnicos; EU/US/JP revisadas.
- [x] Subida de nivel: entrada natural desde un combate de campo revisada
  fotograma a fotograma (sin pop, barras de borde a borde). El fondo que
  reaparecÃ­a a la izquierda al salir era defecto del port: el BG1 del panel
  daba la vuelta al mapa de 256 px; corregido con PortBgClip.
- [x] MenÃº de sÃ­ntesis de salas: panel 6 lÃ­neas por encima de la barra inferior.
- [x] Entradas/salidas de los seis submenÃºs (Ver barajas, Mapamundi, Naipes
  mapa, Naipes mundo, Estado, Diario) revisadas cada 3 fotogramas en EU; la
  salida de Ver barajas coincide con la GBA (el panel de naipes activos
  desaparece de golpe tambiÃ©n en el original). Mapamundi con footer completo.
- [x] Naipes mundo y Mapamundi en US/JP (fixture de sala + modo directo).
- [x] Cabeceras: en US/JP el margen mostraba letras ocultas del sprite (y, al
  pasar el tÃ­tulo a BG, columnas copiadas del cuadro). Ahora la banda usa el
  par de columnas mÃ¡s opaco y sin letras, guardado mientras la cabecera sigue en
  pantalla. Naipes mundo, Mapamundi, Estado, Diario y Barajas revisados en EU/US/JP.
- [x] Tabs del ojo en Estado y Barajas (EU/US/JP) y escenas con estirado
  (Bonus Ã©lite, Tienda moguri, Elige un mundo, VÃ³rtice) revisadas: barra al 100 %.
- [x] Sin estirado de pÃ­xeles: Diario, Elige un mundo, VÃ³rtice, Tienda moguri y
  Bonus Ã©lite usan mÃ¡rgenes hechos por el PPU con sus propias capas: el
  fondo (la capa BG mÃ¡s baja) sigue en espejo con sus tiles y paletas, como
  el flip horizontal del hardware; las capas de interfaz solo continÃºan como
  bandas donde llegan al borde. Sin tramado ni columnas estiradas.
- [x] Widescreen field: recorrido por mundo completo (arriba).

## Claude: verificaciÃ³n, regiones, texto HD, opciones y cierre del PPU

- [x] Gameplay verification: baterÃ­a de salas (test_rooms.py) 52/52 en EU: cada
  mundo en una planta temprana y una tardÃ­a y los vestÃ­bulos de las 13 plantas.
  Hoja de capturas en build/roomtest_eu/rooms.png para la revisiÃ³n de Codex.
- [x] Gameplay verification: estrÃ©s de submenÃºs (test_submenus.py) sobre una
  partida real: las seis entradas del menÃº de pausa, cuatro idas y vueltas
  cada una, 6/6 sin crash. Pendiente aparte: avisos de tutorial delante del
  texto de las cartas (tutorial de naipes mapa).
- [x] Gameplay verification: stutter de la subida de nivel medido con
  KHCOM_FRAMELOG: 16,7 ms estables, sin tirones ni fotogramas de lÃ³gica perdidos
  (renderizador por software; los picos vistos antes eran carga de otra prueba).
- [x] Gameplay verification: salas de Riku (Reverse/Rebirth) 88/88 en EU y de Sora
  en EU/US/JP 118/120 salas Ãºnicas (las otras dos, un encuentro antes de la captura).
- [ ] Gameplay verification: partida completa de Sora y de Reverse/Rebirth en
  las tres regiones (rutas reproducibles y problemas anotados).
- [x] USA/JP: combates JP 209/209 como Sora y como Riku; eventos 192/194 (los dos
  restantes se cortaron al parar, no fallaron). Barra al 100 %.
- [x] USA/JP: capturas regionales de arenas, salas, Naipes mundo y Mapamundi.
- [x] HD text: inventario (port/progress/HDTEXT_INVENTARIO.md).
- [x] HD text: nÃºmeros de barajas y EXP en HD con su color y tamaÃ±o (barra al 100 %).
- [x] Graphics options menu: test_options.py, 35 casos y el menÃº en EU (cinco
  idiomas), US y JP. Barra al 100 %.
- [x] PPU: regresiÃ³n de escenas (test_ppu.py, 12 escenas con referencia
  revisada). Barra al 100 %. Si un cambio de composiciÃ³n altera una escena,
  revisarla y regrabar con --update.
- [ ] PreservaciÃ³n: reproducir los casos de BUGS_GBA.md en EU/US/JP y separar
  tÃ©cnicas originales, anomalÃ­as del cÃ³digo y defectos del port.
- [x] Launcher e instalaciÃ³n: baterÃ­a 11/11.
- [x] Audio: comparaciÃ³n con emulador (mÃºsica, vÃ­deos), baterÃ­a de las 637
  canciones (port/progress/songs_report.txt), detalles del PSG, audio suave.
- [x] Estados guardados deterministas (3/3).
- [x] Releases v0.5, v0.6 y v0.7 (pre-releases pÃºblicas).
- [~] QoL: devolver cartas acumuladas y bancos de partidas hechos; aviso de
  puntos moguri, revancha de jefes y repetir tutoriales pendientes (no suman
  barras; despuÃ©s de las barras).
- [~] HD-2D experimental (versiÃ³n alternativa): profundidad de campo, bloom y
  gradaciÃ³n publicados; el pop-up de diorama estÃ¡ en un stash (bded017)
  y vuelve despuÃ©s de las barras.

## Reglas compartidas

- No modificar a la vez los mismos archivos; Codex trabaja principalmente en
  map/map_tasks/map_cell, card_level_up, perfiles de menÃº, PPU y presentaciÃ³n.
  Claude trabaja en harness y pruebas, opciones, launcher, audio y texto HD.
- Cada uno hace commit solo de sus propios cambios (por bloques si un archivo es
  compartido) y no toca el trabajo sin commitear del otro.
- No sobreescribir ejecutables o estados que el usuario estÃ© usando. La build
  compatible con sus estados estÃ¡ en build/priority-reference; las ranuras
  3/4/7/8/9 recientes en build/pcwork/states.
- Probar en directorios aislados y con pocos procesos mientras el usuario juega.
  No modificar partidas/configuraciÃ³n del usuario.
- No subir ROM ni recursos extraÃ­dos al repositorio.
- Subir porcentajes solo con evidencia y distinguir implementado de verificado.
- Usar emuladores Ãºnicamente para contrastar bugs o comportamientos dudosos del
  original. Validar acabados visuales y transiciones directamente en el port.
