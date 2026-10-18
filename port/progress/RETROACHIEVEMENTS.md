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
trucos, revanchas, savestates y modificadores de gameplay deshabilitan nuevos
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

## Corrección de la primera aproximación

El primer lector solo leía direcciones de bus y no bastaba para integrar RA.
Ahora PortRaReadMemory interpreta el mapa aplanado oficial GBA: 0x00000-0x07fff
IWRAM, 0x08000-0x47fff EWRAM, 0x48000-0x57fff SRAM. Incluye cruces entre ventanas,
rechazo de exceso y de overflow. Esto aún NO proyecta los globals del port: el
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

Builds Windows EU/US/JP y launcher: build/ra-{eu,us,jp}. La ROM EU local coincide
con d369f791d86a969778d2c3a5278d544c, admitida por el juego 768. La lista oficial
solo registra ese archivo y 02c38fb19dab302379ece65724fb4e0f para Level 1; US/JP
usan los retos locales. No se fuerza el hash EU para otra región o una ROM modificada.
La huella se calcula antes de aplicar relocaciones nativas a la ROM mapeada.

Pendientes reales: login con cuenta del usuario desde la interfaz; proyección y
validación del set oficial; compatibilidad nativa del parche Level 1 y su hash;
proceso de validación oficial antes de anunciar soporte online/Hardcore. Ninguno
se considera cerrado por tener una interfaz, un mock o los retos locales.

## Fuentes primarias

- https://github.com/RetroAchievements/rcheevos/tree/v12.5.0
- https://github.com/RetroAchievements/rcheevos/wiki/rc_client-integration
- https://retroachievements.org/game/768/hashes
- https://retroachievements.org/game/768?set=40403
