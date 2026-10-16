# Trabajo activo — Android y rendimiento PC v1.9 (16/10/2026)

- [x] v1.8.3 única stable publicada, tags previos conservados, README/capturas actuales.
- [x] Estudio Android, bloqueos nativos y ruta SDL/NDK/launcher SAF: ../android/README.md.
- [x] Diseño táctil GBA, iconos vectoriales propios de rodar/apilar y prototipo HTML.
- [x] build_all.py configura/compila EU/US/JP juntos; compilación incremental verificada.
- [x] Limpieza autorizada: 1.377 BMP generados, 6,50 GiB recuperados. PNG de evidencia,
  informes, ROMs, partidas/estados, ejecutables y tráiler final conservados.
- [ ] Android: backend ARM/ELF, memoria y sustitutos Win32; no hay alpha jugable aún.
- [ ] PC v1.9: benchmark x1/x2/x4, optimizaciones medidas, regresiones y release.

El prototipo táctil aún no está conectado al juego. No anunciar Android disponible.
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

# CODEX: TAREA ACTUAL — 3 bugs para v1.8.2 (16/10/2026, Claude)

Haz SOLO estos 3 bugs, en este orden. No toques zoom, márgenes de menús,
HD HUD, HD-2D, audio ni tráiler. Commits locales en `khcom-claude`
(rama claude/windows-5); Claude integra, publica v1.8.2 y la sube encima
de la release v1.8.2 actual. Al terminar cada bug, anota aquí resultado y
evidencia. El 4º (números HD sobre el submenú de Ver barajas) queda para
el futuro: NO lo hagas ahora.

Verificación obligatoria: lo que ve el jugador es la ventana. KHCOM_SHOTS
guarda el lienzo/ventana del arnés, pero en escenas GBA-size los márgenes
pueden venir del lienzo del PPU (fuera de la columna GBA) y no del
presentador; confírmalo con KHCOM_RECORD=desde-hasta:ruta.mov (lee la
ventana) o comparando con capturas del usuario. Ventana de prueba 1536x864
(la del usuario) y 1920x1080. No publiques nada sin ver el frame exacto.

## Bug A — Transición tras elegir idioma (parpadeo + fondo antiguo)
Reproducir: arnés sin KHCOM_MODE, lang=None, KHCOM_KEYS=200-203:A,
KHCOM_SHOTS=236..260 (cada frame), KHCOM_WINDOW=1536x864.
- Frame 251 (primer frame de mode_copyright1, PortSideStyle COLOR=1):
  centro negro y márgenes BLANCOS -> parpadeo. Frames 250 y 252: todo
  negro / todo blanco (correctos).
- Frames 246-247 (fundido de mode_lang, estilo TEXTURE=2): los márgenes
  repiten los iconos de Sora/corazones de la fila inferior ("fondo antiguo"
  que el usuario ve 2 frames). Causa probable: al oscurecerse, las bandas
  con emblema pasan el umbral "clean" (hi-lo<=64, croma<=40) y se copian.
- KHCOM_DBGSIDE=1 (ya en el código) loguea por frame: style, fit, wide,
  src. En 251: style 1, fit 0, wide 0, src 72,28 240x160 (lienzo 384x216).
- Intento de Claude ya en el código, en port_main.c (presentador):
  UniformPicture() pinta los márgenes con el color plano cuando la imagen
  GBA es uniforme, y SideBar TEXTURE usa umbrales relativos al contraste.
  NO cambió las capturas -> los márgenes vistos salen de otra parte
  (lienzo PPU fuera de 72..312 o del primer frame del modo nuevo con
  estilo/datos del anterior). Encuentra quién pinta esos píxeles y arréglalo
  ahí.
- Objetivo: fundido suave, sin ningún frame con márgenes de distinto color
  que el centro ni iconos repetidos.

## Bug B — Margen derecho oscuro al entrar a Cargar desde el título
Captura del usuario: tras elegir CARGAR en el título, durante el fundido a
negro el margen derecho queda casi negro mientras el izquierdo sigue gris
(igual que el centro). Ventana 1536x864, partida existente (save=True).
Reproducir: KHCOM_KEYS con A 200..800 cada 60, START 1120, luego DOWN y A
sobre CARGAR cuando aparezca el menú (~frame 1400-1430) y capturar cada
frame del fundido siguiente. Probablemente el mismo mecanismo que el Bug A
(margen que no sigue el brillo del centro). Objetivo: ambos márgenes con
el mismo color/brillo que el borde de la imagen en cada frame.

## Bug C — Destello del cuarto anterior al cambiar de sala
Usuario: puerta arriba-derecha (isométrico) en su partida; state en
ReKHCoM-v1.8.1/ReKHCoM-v1.8.1-windows-x86/states/slot1.khst (SOLO carga
con build/v181/khcom.exe = exe de la release v1.8.1, por BuildId).
Con ese exe y KHCOM_LOADSTATE: el fundido a blanco termina ~frame 36-38;
frame 39 muestra pilares/objetos del castillo en BLANCO sobre NEGRO.
KHCOM_PPULOG en ese frame: "latch 0110" (BG3 pierde el latch del mundo un
frame antes que BG1/BG2), siguiente frame dispcnt 0040 (BGs apagados).
Intento de Claude (en v1.8.2, NO verificado): gracia de 1 frame en
PortPpuBeginFrame que mantiene el latch del BG que lo pierde. Verificar
con un state nuevo: el usuario puede guardar uno con v1.8.2 frente a la
puerta, o reproducir saliendo del castillo por una puerta con el arnés.
Objetivo: ningún frame con siluetas del cuarto anterior; el fundido pasa
de blanco a la sala nueva sin destellos.

## Después de los bugs: tráiler v4
Sigue `port/progress/TRAILER.md` (pasos, scripts en `Trailer/tools/` y
mejoras pedidas). Solo cuando los bugs A-C estén publicados.


# Abiertos tras v1.8.2 — 16/10/2026 (Claude). El usuario confirma que NO están corregidos en v1.8.2.

1. Ver barajas: al abrir el submenú, los números/textos HD siguen encima del
   menú. Intento v1.8.2: recorte por columnas en DrawSmallNumbers (solo
   estilo 2, solo BG); pasó en el arnés a 1536x864 pero no en el juego real.
   Revisar también glifos de estilo 0 y el orden capa HD vs. sprites del menú.
2. Cambio de sala (puerta arriba-derecha de la partida slot 1, v1.8.1
   states/slot1.khst): 1 frame con siluetas blancas del cuarto anterior.
   Causa vista con KHCOM_PPULOG: frame con latch 0110 (BG3 pierde el latch
   antes que BG1/BG2). Intento v1.8.2: gracia de 1 frame del latch; NO
   verificado (el state solo carga con el exe v1.8.1: BuildId).
3. Título -> Cargar: el margen derecho se oscurece durante el fundido.
   No reproducido en el arnés.
4. Arranque: justo después de elegir idioma, 1 frame con los márgenes sin
   pintar (negros) antes de la pantalla siguiente. Probable cambio de modo
   (mode_lang -> copyright) en el que PortSideStyle/margen aún no tiene datos;
   capturar frame a frame desde ~frame 250 con KHCOM_SHOTS.

   Hallazgo (Claude, arnés 1536x864, A en frame 200): fundido de mode_lang
   236-249 con márgenes de textura que no siguen el brillo del centro y que
   repiten los iconos de Sora de abajo (frames 246-247); frame 251 (primer
   frame de copyright1) con centro negro y margen derecho BLANCO. El usuario
   pide además que esa transición sea suave. Prioridad para v1.8.2 junto con
   2 y 3; el punto 1 (números HD) puede quedar para el futuro.

   Estado del intento (Claude, sin publicar): en el presentador (port_main.c)
   UniformPicture rellena los márgenes con el color plano al final de un
   fundido, y el estilo TEXTURE usa umbrales relativos al contraste. NO cambia
   las capturas del arnés: KHCOM_DBGSIDE=1 muestra frame 251 = style 1
   (copyright1), fit 0, src 72,28 240x160, o sea el lienzo es 384x216 y los
   márgenes de las capturas salen del lienzo del PPU (fuera de 72..312), no
   de SideBar. Siguiente paso: ver qué pinta el PPU fuera de la columna GBA en
   escenas no-wide (RenderLine con sInWorld/sLeft) en el primer frame de un
   modo, y verificar con KHCOM_RECORD (lectura de la ventana), no KHCOM_SHOTS.

Regla: quien trabaje (Claude o Codex) documenta aquí los pendientes al
avanzar; si a uno se le acaban los tokens, el otro continúa desde esta lista.
Solo Claude publica releases; Codex deja commits locales para integrar.

# Pendientes tras v1.8.1 — 16/10/2026 (Claude). Solo Claude integra y publica.

**Claude (por solucionar)**
- Zumbido de audio (hecho en v1.8.1): rampa de volumen por canal en el
  mezclador m4a. Queda revisar los pasos por frame del PSG/CGB (canales 1-4)
  en combate: el salto en el límite de frame bajó de 3.2x a 2.7x, no a 1x.
- Verificar el ritmo de frames en un PC real con la partida guardada del
  usuario (combate de Ciudad de Paso): en el arnés el trabajo por frame es
  ~20 ms por el renderer software; comprobar que no haya cortes de audio.
- Tráiler v4 (pausado por el usuario).

**Codex**
- Rendimiento del motor (ver reparto v1.7 abajo): 60 fps en x3+ a 1080p.

# Actualización — v1.8 (16/10/2026)

Pedido final del usuario: publicar v1.8 (la propuesta v1.7.2 no se publicó).
HD solo donde ya correspondía a diálogos/subtítulos/menús; números EXP originales
siempre y en su posición anterior. Selector sin filas de enteras/escalado,
anotaciones x6→íntegra x1 con aviso y x3→íntegra x2. x4 actual predeterminado.
Incluye bordes cinematográficos de las ranuras 1/2/3. Ver RELEASE_V18.md.

---

# Coordinación vigente — 16/10/2026

Claude sin créditos. El usuario autoriza a Codex a integrar el último trabajo,
corregir el audio (ranura 1 de v1.6.7), optimizar audio/gráficos, publicar una
release unificada y retirar releases anteriores conservando los tags.
Este pedido reemplaza temporalmente la exclusividad de integración de abajo.

Codex trabaja en khcom-claude; integrados los tres commits de rendimiento de
codex/ppu-performance. Ver AUDIO_BUZZ_V171.md y RELEASE_V171.md para evidencia.
No tocar el trabajo HUD antiguo sin commitear de khcom ni los guardados reales.

---

# Reparto v1.7 — 16/10/2026 (Claude). Solo Claude integra y publica.

Codex: trabaja en `khcom`, commits locales, sin push ni releases. Claude
integra tus commits en `khcom-claude` (rama claude/windows-5) y publica.
No tocar los archivos de la columna de Claude mientras esté este bloque.

**Codex — rendimiento del motor (prioridad)**
- Objetivo: 60 fps con el lienzo grande (1920x1080 en x1; hoy PPU ~86 ms,
  960x540 ~21 ms; ~40 ns/píxel). Medir con KHCOM_PROFILE=1 (log
  "profile: canvas WxH ppu X ms present Y ms" cada 240 frames).
- Ideas: RenderLine/ComposeLine por franjas en hilos (SDL_Thread, sin
  estado compartido por línea), saltar BGs/columnas transparentes, tablas
  de tiles precalculadas, evitar trabajo por píxel fuera de sLeft/sRight.
- Archivos: port/src/port_ppu.c (solo el render por línea; no cambiar
  semántica de márgenes/headers/HUD), port/tools/ppu_conformance.c.
- Aceptación: capturas idénticas píxel a píxel antes/después (batería de
  menús, campo, combate a 1920x1080 y 5120x1440) y tiempos del perfil.

**Claude — interfaz y escalado**
- Zoom x1..x6 (vista en múltiplos de la GBA; x1 Original, x2 Mejorado por
  defecto) con opción Escala entera ON/OFF.
- Parpadeo de las extensiones laterales durante el slide de submenús
  (Mapamundi, Estado...; el Diario no lo tiene).
- Relación de aspecto: opción para forzar ultrawide (21:9 / 32:9).
- Resolución completa: navegar y aplicar con A.
- Opciones por encima del HUD (hecho).
- Archivos: port_main.c, port_ui.c, port_config.c, decompui.cpp, src/*.

# Reparto tras v1.6.7 — 16/10/2026 (Claude)

Publicada v1.6.7 (única release pública). Zoom = escala de píxel entera
(gPortConfig.zoom: 0 Mejorado = x4 a 1080p; N = xN), buffers del lienzo en el
heap (PORT_MAX 2560x1440). Arnés: KHCOM_RESIZE=frame:WxH, KHCOM_PROFILE=1.

**Codex**
1. Rendimiento del PPU por software (~40 ns/píxel): x1/x2 a 1080p no llegan a
   60 fps (86 y 21 ms). Optimizar ComposeLine/DrawTextBg o hilos por franjas.
2. Guardar partida: el usuario vio paneles cortados en v1.6.5/6 (captura con el
   castillo de fondo). No se reproduce en v1.6.7 a 1920x1080 ni 1536x864 con
   zoom 0/1/2/5; confirmar con su partida real y otras salas.
3. HD HUD y HD-2D siguen ocultos y forzados a 0 (KHCOM_HD_HUD_DEV / KHCOM_HD2D_DEV).

**Claude**
1. Tráiler v4 (pausado por el usuario): intro de consola en el Castillo del
   Olvido, salas encadenadas a media escalera, post-créditos Sincorazón ya hecho
   (Trailer/clips/16_postcreditos_android.mov). Regrabar clips con v1.6.7.
2. Revisión de bugs que reporte el usuario.

# HD HUD: pendiente — 15/10/2026 (Claude)

El HD HUD queda **desactivado para los jugadores hasta estar completo**: la
opción está oculta en el juego y en el launcher, y `PortConfigLoad` lo fuerza a
0 aunque `kh_com.ini` diga `hd_hud=1`. Para desarrollarlo y probarlo, arrancar
con `KHCOM_HD_HUD_DEV=1` (entonces vale `hd_hud` del ini). Su trabajo de v0.8
rompió el texto HD de los diálogos de campo (las ventanas de mensaje marcaban
su zona como cubierta); arreglado en v1.4 en `PortPpuTextGlyphs`.

# Codex retomó — 14/10/2026, 08:47 (Santiago)

Objetivo vigente: trabajar junto a Claude hasta verificar todas las barras
al 100 %. Claude mantiene integración y releases. No publicar ni sustituir
partidas/ejecutables del usuario desde este bloque.

## Reparto al retomar

- Claude está modificando `port_ppu.c`, `src/engine.c`, `engine.h` y
  `REPARTO.md` en `khcom-claude`: conserva el cierre de headers/transiciones
  de menús. Codex no toca esos archivos mientras esté ese trabajo activo.
- Codex retoma **Gameplay verification**: contraste del replay VBM USA con
  el cartucho, sincronía tras la intro y rutas reproducibles de ambas
  campañas. Trabaja en herramientas de replay y evidencia independiente.
- El árbol `khcom` está limpio en ef3a82f3b; los cambios de la sesión anterior
  ya fueron integrados por Claude. No reintroducir versiones antiguas.
- `status.json` solo deja abiertas menús/eventos (93 %) y gameplay (85 %).
  El cuadro antiguo de REPARTO y varios informes históricos conservan
  porcentajes anteriores; leer el estado actual y sus commits de evidencia.

### Avance posterior

- Claude integró los headers y cerró menús/eventos en ae59ef83a/22733e166.
  No repetir ese trabajo. Gameplay sigue en 85 %.
- La auditoría `HDTEXT_CIERRE_AUDIT.md` identificó familias numéricas de texto
  aún sin convertir pese al 100 % de la barra. Codex toma exclusivamente
  category/value/CP de los tres archivos de barajas y `mode_status.c`;
  pruebas regionales off/on y comparación exacta del modo pixel en curso.
  Map/save, números OBJ del mapa, teclado de 64 bytes y subida de nivel
  permanecen pendientes, distintos de los números artísticos de naipes.
- `WIDESCREEN_CIERRE_AUDIT.md` no encontró un defecto nuevo de extensión;
  sí un error de contabilidad de las salas: 120 casos únicos por región,
  118 capturas de campo y dos encuentros, no 131/129. Esto no reemplaza
  las partidas completas de Sora/Riku pendientes.

## Lo que queda por demostrar en gameplay

El replay local USA de 20.000 frames terminó sin crash pero seguía en el
primer combate tutorial, nivel 1. Esto no demuestra una partida completa.
La cinta tiene 283.693 frames; antes de ejecutarla completa hay que contrastar
la misma secuencia en emulador y encontrar el primer punto de divergencia.
No inyectar progreso ni convertir batallas/eventos de debug en una campaña.

El contraste original mGBA también se desincroniza respecto de la cinta
VBA-rr: permanece en selección de archivo hasta el siguiente grupo de
inputs, mientras el port aceptó la A anterior. Por eso el tutorial diferente
en frame 20.000 no demuestra por sí solo un bug del port.

`test_tas_boot_sync.py` observa los modos originales mediante punteros de
IWRAM, sin escribir memoria. Evidencia
`build/codex-tas-boot-sync/20261014_085720_415173`: copyright2 408, título 605,
selección de partida nueva 931, todavía ahí en 1600. El mismo prefijo nativo
entra en copyright2 394, título 591, selección 820, película 948. Mover la
cinta 14 frames no resuelve el desfase de navegación: película 934. No
modificar el juego para hacer coincidir a la fuerza un TAS de otro core.

El siguiente paso es adaptar las entradas normales con checkpoints de solo
lectura. Riku tiene además un archivo SRAM original desbloqueado extraído
byte a byte de la cinta BK2 (no se editan estadísticas ni progreso Riku);
el probe inicia una campaña Riku nueva por el menú y llega al primer combate.
La continuidad y el final de ambas campañas siguen sin demostrar.

Las herramientas `test_gameplay_*` quedan como evidencia de salas, submenús,
pacing y replay; `test_rooms`, `test_submenus`, `test_ppu` y `test_options`
de Claude se conservan. Sus alcances son diferentes y no se borran por
duplicidad nominal.

Al terminar un bloque, Codex hace commit solo de sus archivos propios para
que Claude pueda integrarlo. Las capturas/ROMs/cintas/partidas permanecen en
directorios locales ignorados.
