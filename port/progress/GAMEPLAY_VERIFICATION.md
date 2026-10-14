# Verificación de gameplay — 14 octubre 2026

## Alcance y criterio de cierre

La barra Gameplay verification no puede declararse 100 % con entradas de
depuración, capturas ni ausencia de crashes solamente. Requiere recorrido
completo natural de Sora y Reverse/Rebirth en EU/US/JP, más salas, submenús y
pacing. Se conservan partidas, estados y ejecutables del usuario. Las pruebas
corren en carpetas nuevas `build/codex-gameplay-*`, sin SRAM heredada.

## Herramientas añadidas

- `src/main.c`: fixture `room` evaluado contra `Mode_MapChk_1`; se inicializa
  realmente Sora. `roomriku` inicializa realmente Reverse/Rebirth y su deck,
  en lugar de presentar a Sora con otro escenario. Se validan modo, mundo y
  planta antes de indexar tablas. `roomgraph` enumera por BFS la topología
  alcanzable desde la entrada; `roomgoto` rechaza salas fuera de esa topología.
  `progress` registra estado sin modificarlo.
- `test_gameplay_rooms.py`: entradas y capturas, `--all-rooms` recorre las salas
  alcanzables de los grafos originales. `--world`, `--floor`, `--matrix`,
  `--halls`, `--actor`, `--version`, `--scale`. Cada caso debe tener captura,
  marca de fixture y modo esperado vigente. El trace registra solo cambios de
  modo; no exigir una transición nueva si ya estaba en el mismo modo.
- `test_gameplay_menus.py`: inicialización de campo por fixture MAP; desde allí
  START/DOWN/A reales abren cada submenu, navegan pestañas y vuelven al campo.
  Exige entrada al modo correcto y retorno posterior, además de ausencia de
  crash. Reverse/Rebirth excluye Mundo/Diario deshabilitados.
- `test_gameplay_tas.py`: replay de VBM local USA desde encendido, sin inyectar
  progreso. Registra checkpoints read-only y capturas. `exit=0` nunca implica
  partida completa: `full_story_verified` se mantiene false hasta comprobar
  final y sincronía. No copia ni distribuye ROM o VBM.
- `test_gameplay_level_timing.py`: crea y copia un estado aislado en recompensa;
  en proceso nuevo omite **completamente** `KHCOM_FAST`, mide 600 intervalos
  después del calentamiento y exige avance de lógica de uno por fotograma.
  La instrumentación anterior `test_levelup.py --timing` usaba `KHCOM_FAST=0`,
  pero `Pace()` comprueba presencia de la variable, por lo que esa medición no
  estaba a ritmo normal. El coordinador corrige ese hunk conservando el WIP.

## Evidencia obtenida

EU, Sora, 13 entradas del MAP debug, planta 1, vista automática, HD text,
1280×720: `build/codex-gameplay-rooms/20261014_034832_461231`.
13 capturas, exit 0, sin CRASH; modo `Mode_MapFld` vigente en todas.
Primer informe marcó 3/13 por una assertion incorrecta del trace, corregida y
recalculada desde evidencia preservada sin repetir la ejecución.

**Límite:** fixtureworld12 es FORMAT (gameworld0), no un mundo de historia.
fixtureworld10 es Twilight Town (gameworld11), cuya planta1 genera un
placeholder/debug. Estas dos capturas no prueban un escenario jugable final.
La tabla de MAP no incluye 100 Acre Wood. Hace falta cubrirlo por ruta propia.

USA, Sora, replay natural de los primeros 6000 frames del VBM local:
`build/codex-gameplay-tas/20261014_035414_316703`. Exit 0, sin CRASH,
copyright → title → selección de partida nueva → opening. Frame6000 muestra
el castillo al final del opening, todavía `mode_movie`, nivel1. Esta evidencia
valida entrada natural e inputs iniciales, todavía no combate ni final.
VBM: 283693 frames; SHA-256
`7e2871e84b7c8d391030f36f825bcf8f5699c8c5e6a309d8ac876d1949208403`.

USA O2, recompensa idle, automática, HD, 640×360, ritmo normal real:
`build/codex-gameplay-level-timing/20261014_040141_619861`, PASS. 600 intervalos,
avance de lógica exactamente 1; p50 16.7427 ms, p95 16.7437 ms, p99 16.7566 ms,
máximo 16.9161 ms, ninguno supera dos periodos de GBA (33.4854 ms).

Original view0, mismo caso y ritmo normal:
`build/codex-gameplay-level-timing/20261014_040233_414097`, FAIL de pacing,
600 intervalos y lógica exactamente1; p99 58.4136 ms, máximo118.2098 ms,
16 superan dos periodos. Todos los picos están en frames661–692, mientras la
pasada automática inmediatamente anterior fue estable. Coincidió con carga
de host/pruebas concurrentes: **no atribuir todavía al menú**; repetir sin
otras builds o procesos antes de diagnosticar. SDL dummy mide CPU, no GPU,
display ni VSync del usuario.

## Pendientes que impiden 100 %

- Ejecutar y revisar todas las salas reales y cámaras, incluidos mundos fijos,
  100 Acre Wood y Twilight Town en la planta apropiada.
- Ejecutar batería ampliada de submenús y edición, incluidas capas de tutorial.
- Medir subida de nivel a ritmo normal con intervalos y avances de lógica.
- Confirmar sincronía del TAS USA tras la intro y a lo largo de la historia;
  su cinta Sora USA no prueba por sí sola EU, JP ni Reverse/Rebirth.
- Documentar recorridos naturales completos y sus finales en ambas campañas,
  tres regiones, sin sustituirlos por las pruebas de eventos/jefes existentes.

No se aumenta Gameplay verification desde 66 % con esta evidencia parcial.
