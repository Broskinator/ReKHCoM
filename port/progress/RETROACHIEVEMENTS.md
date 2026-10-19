# Logros locales y RetroAchievements

Estado al 18/10/2026. Responsable: Codex, punto 27 del reparto.

## Implementación disponible

- Windows: rcheevos v12.5.0, MIT, fijado al commit
  1433173220a7eaede6a9ed7a18e94117be1821e0. Fuentes y licencia en
  port/third_party/rcheevos; licencia incluida en paquetes Windows.
- Launcher: pestaña Logros con el esquema blanco/negro/rojo existente, filtros,
  búsqueda, selección EU/US/JP, progreso local, cuenta y catálogo oficial.
- Cliente HTTPS WinHTTP en workers: los callbacks se entregan en el hilo dueño,
  hasta cuatro peticiones simultáneas, límites de respuesta, sin redirecciones
  de credenciales y sin guardar contraseñas. Recordar cuenta guarda exclusivamente
  usuario/token cifrados con DPAPI para el usuario Windows. Cerrar sesión borra
  ese token. La conexión es optativa y conserva su ajuste.
- El catálogo oficial y los desbloqueos de cuenta se consultan por separado en
  modo espectador, sin abrir sesión de juego ni enviar premios, puntuaciones o
  presencia. La evaluación oficial en el juego está desactivada expresamente.

## Local: EU, US y JP

16 retos propios del port, con nombres/descripciones ES/EN, usan directamente
GameState y PlayerProgression nativos, sin depender de direcciones de la ROM EU.
Incluyen niveles, HP/CP, moguri, amigos, clases de cartas, trucos, pisos de Sora y
finales de Sora/Riku. No son una conversión de los 90 logros oficiales ni conceden
puntos a una cuenta RA. Se guardan por región en achievements_eu.dat,
achievements_us.dat y achievements_jp.dat mediante reemplazo atómico.

La primera observación válida establece una base: un save previo no concede
logros retroactivos. Se necesitan transiciones observadas durante el juego. Los
trucos, menú debug, revanchas, savestates y modificadores de gameplay deshabilitan nuevos
premios hasta reiniciar el juego; apagar el truco no restaura integridad en esa
sesión. Los harness de pruebas tampoco conceden premios. El ajuste local vive
en achievements_local.ini y se aplica al iniciar el juego.

## Avisos y resolución

Cola de hasta 16 avisos, arriba al centro de toda la salida presentada. Blanco,
negro, acento rojo y fuente original del juego. Entrada de 300 ms con desplazamiento
y fundido, 3.9 segundos de lectura, salida de 300 ms. Se adaptan por escala entera
a la altura de salida, independientemente del zoom de la escena; el texto largo
se abrevia y una ventana pequeña conserva margen. No roban input ni se repiten.
La pestaña permite probar una vista previa sin conceder un logro.

Capturas: 1080p con zoom x3, build/ra-toast1080. La variable de harness
KHCOM_ACHIEVEMENT_PREVIEW=180 muestra un aviso claramente marcado VISTA PREVIA,
sin escribir progreso. La presentación funciona también en los backends SDL
portables; cliente de cuenta/HTTPS actual: Windows.

test_achievement_presentation.py también renderiza la salida real a 240x160,
640x360, 1920x1080 y 2560x1080 y comprueba centrado/márgenes, capturando entrada,
lectura y salida. Ninguna vista previa concede progreso. Evidencia y clip MP4 en
build/achievement-presentation. Esa grabación usa reloj de frames únicamente
para el aviso marcado VISTA PREVIA, evitando que el coste del encoder cambie su
animación capturada; durante el juego normal se usa tiempo real SDL.

## Corrección de la primera aproximación

El primer lector solo leía direcciones de bus y no bastaba para integrar RA.
Ahora PortRaReadMemory interpreta el mapa aplanado oficial GBA: 0x00000-0x07fff
IWRAM, 0x08000-0x47fff EWRAM, 0x48000-0x57fff SRAM. Incluye cruces entre ventanas,
rechazo de exceso y de overflow. La revisión inicial no proyectaba los globals del port: el
juego compila muchas variables en khdata/khbss y algunos punteros/layouts no
coinciden con la RAM del cartucho. Evaluar esas posiciones como si fueran
correctas podría conceder premios falsos. El modo oficial seguirá sin evaluación
hasta validar cada referencia y cadena de punteros del set contra el runtime.

## Evidencia y límites

python port/tools/test_achievements.py pasa: mapa GBA, límites y overflow;
transiciones locales, no retroactividad, persistencia y corrupción; puente real
PortAchievementsFrame en EU/US/JP; invalidación por modificadores; seis tamaños
de layout del aviso (240x160 a 3840x2160); login rechazado, token cifrado,
reconexión y logout; hash EU real y carga/evaluación de un catálogo sintético a
través de rcheevos, sin envíos de premios ni sesión. Fixtures no son un login real.
El informe reproducible queda en build/achievements-test/report.json.

Builds Windows EU/US/JP y launcher: build/ra-{eu,us,jp}. Copia ejecutable para
revisión local en build/achievements-review/ReKHCoM.exe, sin modificar la
instalación ni los guardados del usuario. Las fuentes nuevas del puente local,
aviso y lector pasan también compilación con el perfil guest x32 de PS5. La
persistencia evita dependencias de fscanf/strtoull/time en ese guest; no es una
prueba de ejecución física de PS5 ni un paquete publicado.

La ROM EU local coincide
con d369f791d86a969778d2c3a5278d544c, admitida por el juego 768. La lista oficial
solo registra ese archivo y 02c38fb19dab302379ece65724fb4e0f para Level 1; US/JP
usan los retos locales. No se fuerza el hash EU para otra región o una ROM modificada.
La huella se calcula antes de aplicar relocaciones nativas a la ROM mapeada.

El usuario confirmó el login real desde la interfaz el 18/10. La captura nueva recupera la cuenta recordada y consulta el catálogo y su progreso sin enviar premios.

Pendientes reales: proyección y
validación del set oficial; compatibilidad nativa del parche Level 1 y su hash;
proceso de validación oficial antes de anunciar soporte online/Hardcore. Ninguno
se considera cerrado por tener una interfaz, un mock o los retos locales.

## Fuentes primarias

- https://github.com/RetroAchievements/rcheevos/tree/v12.5.0
- https://github.com/RetroAchievements/rcheevos/wiki/rc_client-integration
- https://retroachievements.org/game/768/hashes
- https://retroachievements.org/game/768?set=40403


## Revisión del 18/10: catálogo directo e imágenes

La página Logros presenta el catálogo completo con desplazamiento, búsqueda y
filtro, sin entrar a submenús. Cuenta/inicio de sesión queda en un desplegable.
Las imágenes oficiales se descargan con el cliente HTTPS y se cachean; se generan
texturas gris/color de la misma imagen y el estado procede del progreso real de
cuenta. El catálogo público también se conserva sin conexión; sin consulta de
cuenta se indica progreso desconocido. La captura de revisión recibió 91 entradas
(el catálogo es dinámico; el número anterior era 90). Los 16 retos locales usan
sprites de cartas del cartucho instalado, sin distribuirlos con el programa.

Prueba separada: `../Probar logros 27.bat`. No incorpora credenciales en releases.
Capturas privadas: build/ra27-review y build/ra27-earned. Matriz de comprobación:
LOGROS_27_VALIDACION.md. No se declara compatibilidad oficial online por pasar
pruebas locales ni por mostrar desbloqueos obtenidos previamente en otra aplicación.


19/10: Opciones > Logros añade una consulta in-game de pendientes/conseguidos y
filtros. Cuenta/login permanecen exclusivamente en launcher. El catálogo público
y una instantánea mínima del último progreso consultado alimentan la lista
official; los estados locales se leen del runtime. Ver LOGROS_27_VALIDACION.md.


## Proyección escalar de GameState (19/10)

La raíz del bloqueo oficial es que RAM mapeada y globals nativos son memorias
distintas. port_ra_state.c superpone los tramos de GameState sin punteros al mapa
aplanado que lee RA. Las bases originales se verifican en las instrucciones LDR r4
de InitGameState de cada cartucho: EU 0x0203A1C0, US 0x02039BB0, JP 0x02039B20.
Se proyecta el prefijo escalar y desde progression hasta el final; se excluye
explícitamente enemyCache y su pool, que contienen punteros nativos.

Pruebas: test_ra_state_addresses.py verifica las bases contra ROM privada;
test_ra_state.c verifica bytes, límites, lecturas parciales, overflow y exclusión
de punteros en x86 y Wasm por región. Puente y menú siguen pasando compilación en
Linux/Android/PS5. El parser local acepta LF y CRLF también fuera de Windows.

Esto no completa la proyección del estado de batalla, las cadenas de punteros ni
la validación de los retos oficiales. Siguen desactivados evaluación oficial,
premios online y Hardcore; no se afirma compatibilidad por añadir este primer
tramo de memoria. El cliente no envía sesiones ni premios durante las pruebas.


La misma proyección incorpora barajas, colección de cartas, cantidad, cartas de
mapa e índice de baraja activa. Son datos sin punteros; sus seis bases por región
se verifican en los LDR de InitGameState, GetActiveDeck, InitCardCollection,
CountOwnedMapCardKinds y GetActiveDeckIndex. Deck mantiene sizeof == 0xE0.

Auditoría real del set: audit_ra_references.py --data-dir <copia conectada>
--rom <ROM EU> exporta referencias públicas de las condiciones usando la cuenta
ya conectada, en modo espectador y sin evaluar frames. --offline resume un CSV
existente sin red. No exporta usuario, token, contraseña, desbloqueos ni valores
RAM. Depende de la revisión fijada de rcheevos para recorrer sus estructuras.

Resultado privado de esta revisión: 91 entradas de catálogo (90 grafos con
referencias y un aviso del sistema); 26.120 referencias recorridas, 157 direcciones
directas distintas y 139 dentro de los tramos escalares proyectados. Quedan 18
direcciones directas y la validación de las referencias modificadas/indirectas.
Estos números no son porcentaje de logros funcionando: ningún logro oficial se
habilita hasta validar el grafo completo y los objetos apuntados. El informe
build/ra-reference-audit/summary.json marca official_evaluation=DISABLED.
