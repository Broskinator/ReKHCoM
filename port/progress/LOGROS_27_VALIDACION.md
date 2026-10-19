# Logros 27: matriz de comprobación — 18/10/2026

## Qué se comprueba

- `python port/tools/test_achievements.py`: los 16 retos locales mediante
  `PortAchievementsFrame`, usando los campos reales de `GameState`, en EU/US/JP.
  Cada transición emite un aviso y persiste en el archivo de su región; repetir
  el frame no repite el aviso. Alterna modos mapa/batalla. Prueba aparte de
  invalidación por trucos, incluso después de desactivarlos.
- El núcleo también comprueba cada umbral por separado, no retroactividad al
  cargar una partida avanzada, condiciones exclusivas de Sora, ambos campos de
  trucos aprendidos, persistencia y archivo corrupto.
- `python port/tools/test_achievement_targets.py`: ejecuta las mismas condiciones
  en WebAssembly/Node y compila el puente y el presentador reales para cada
  región con los toolchains de Linux i386/ARMhf, Android ARMv7/x86 y guest PS5 x32.
- Informes generados en `build/achievements-test/report.json` y
  `build/achievement-targets/report.json`. Las pruebas usan datos sintéticos y no
  modifican partidas del usuario ni envían premios a RetroAchievements.

| Destino | Disparos locales | Presentación / integración | Hardware real |
|---|---|---|---|
| Windows EU/US/JP | 16/16 por región en puente nativo | Launcher e imágenes revisados; avisos con harness SDL | No es una partida completa manual |
| Web | 16/16 en Wasm | Pestaña local directa; gris/color; pide IDBFS al desbloquear | Revisión del navegador separada del harness |
| Linux / Steam Deck i386 | Fuente del núcleo compartida | Puente y avisos compilan EU/US/JP | Pendiente de dispositivo |
| ARMhf / Anbernic compatible | Fuente del núcleo compartida | Puente y avisos compilan EU/US/JP | Pendiente de dispositivo |
| Android ARMv7/x86 | Fuente del núcleo compartida | Puente y avisos compilan EU/US/JP | Sin dispositivo conectado en esta revisión |
| PS5 guest x32 | Fuente del núcleo compartida | Puente y avisos compilan EU/US/JP | Pendiente de consola |

La compilación comprueba tipos, dependencias y ABI; no demuestra que el guardado,
el aviso ni el disparo se hayan ejecutado en una consola física. No se marca esa
validación como completada ni se interpreta como soporte de trofeos PSN.

## RetroAchievements oficial

Login real confirmado por el usuario; consulta real de catálogo y progreso en
Windows. La lectura del mapa GBA aplanado pasa sus límites, pero la proyección de
los globals/punteros del port al set oficial sigue pendiente. La evaluación y
envío de premios oficiales y Hardcore permanecen desactivados en todas las
plataformas. US/JP tienen retos locales, sin falsear la huella de EU.

El resto de ports no incorpora todavía el transporte WinHTTP/DPAPI de Windows:
no se presenta el acceso de cuenta como disponible allí. Esta matriz no cierra
el punto 27 completo; registra qué partes están comprobadas y qué falta.


## Opciones > Logros (19/10)

Listado de consulta dentro del juego, sin login ni acciones de cuenta. Incluye
los 16 retos locales de la región actual y, si el launcher ya cargó el catálogo,
la lista oficial EU. Arriba/abajo navega, izquierda/derecha filtra todos,
pendientes o conseguidos; Tab/Y cambia local/oficial y B vuelve a Opciones.

El launcher guarda la última consulta de estados en cache/achievement-unlocks.dat,
sin usuario, contraseña ni token. El juego la trata como una instantánea y lo
indica en pantalla; si no existe o está truncada muestra «Sin consultar». No
convierte esa instantánea en premios ni en progresión local. El cierre de sesión
invalida el estado de consulta. No se incluyen archivos de cuenta/cache en releases.

El menú usa las barras blancas, selección roja y animación de luz existentes.
El parser y el menú tienen límites fijos (máximo 256 entradas públicas) y se
cargan al abrir la lista. El guardado local sigue usando el archivo por región.
Prueba reproducible: test_achievement_menu.py, con navegación mediante el mismo
handler SDL, fixtures aisladas y comprobación de que consultar no cambia logros
ni crea una partida. No necesita VM ni emulador.

Durante la sesión de juego de 2 horas pedida por el usuario: compilar con -j1,
prioridad BelowNormal y no iniciar VM/AVD/navegadores con motores Wasm cargados.
