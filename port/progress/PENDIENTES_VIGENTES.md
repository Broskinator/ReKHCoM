# Cola vigente — pasada al limpio, 18/10/2026

Única cola de trabajo (los demás .md son evidencia). Numeración histórica
conservada para poder rastrear. Reglas del usuario:

- Claude y Codex commitean/pushean a main; si uno lleva más de 10 min sin commits ni
  archivos tocados, el otro toma sus puntos anotando hora y evidencia aquí.
- Todo lo visual se entrega como copia jugable + .bat y espera aprobación del
  usuario; lo aprobado se documenta en LOGICA_APROBADOS.md; método: PRUEBAS_VISUALES.md.
- Lo publicado sale de la cola. Validación en hardware real NO es pendiente:
  llega como reporte (sección final).
- Nunca borrar carpetas ReKHCoM-vX sin pedirlo el usuario (18/10: autorizó borrar
  las antiguas dejando 3; respaldo de guardados en backups/).

## 1. Esperan la aprobación del usuario

- (vacío) 49 mando: aprobado por el usuario el 18/10 y publicado en v1.9.4.
- **12 (Codex):** 057 playa aprobada con mejoras (ver sección 2).

## 2. Codex

- **12:** 057 playa: igualar la estructura de tiles del lado superior derecho
  (junto a Sora) con la del izquierdo; el pasto no debe tener motivos cortados.
- **16–24:** arte de jefes/escenas, al final y de a uno con antes/después:
  16 Darkside (WIP en rama wip/darkside-backdrop-20261018, fuera de main),
  17 Oogie, 18 Jafar, 19 Maléfica dragón, 20 Úrsula, 21 Marluxia final,
  22 Alicia/Trickmaster (tablero inferior), 23 resto de candidatas,
  24 tira de fondo de arenas GBA.
- **26:** issue #2 (funciones del remake): respondido el 18/10; esperar la lista
  de 1–2 escenas concretas del autor. No prometer remake.
- **27:** RetroAchievements: hay logros locales y catálogo oficial experimental;
  falta login real y validar memoria/set; Level 1 pendiente.
- **29:** variantes de planta de Mapamundi y objetos de sala en transiciones.
- **36–37:** Web en navegador real (arranque, combate, SRAM, texto HD, audio, FPS,
  controles); Worker/OffscreenCanvas solo si mejora medida.
- **40:** audio PSG frente a la referencia de combate.
- **2:** recoger el cierre del chat del bug de sincorazones.

## 3. Claude

- **39 (al final):** rutas completas Sora y Reverse/Rebirth. Método aprobado:
  speedrun (.vbm USA en la raíz) como guía por tramos, comparar con el emulador
  en puntos de control (sala, mundo, HP, escena), corregir el desfase por lag,
  replicar y verificar cada escena; Reverse/Rebirth con el bot (KHCOM_BOT).
- **44:** limpieza: hecha la de carpetas ReKHCoM-vX; falta worktrees y carpetas
  build viejas (preguntar antes de borrar).
- **43:** mantener esta cola y los contadores tras cada cierre.

## Cerrados hoy (referencia)

Guardados y opciones (5–9), #17 borde de Riku (10), #18 Mapamundi (11), 061 hielo
y 074 Monstro (13–14), remapeo y gatillos (25), submenús (28), fuente CJK Linux
(34), PS5 (38), táctil (45), transición de Opciones (50), regresión sincorazones (4).
Publicado en Nightly 5 (18/10, v1.9.3-nightly.20261018.5): #17 borde de Riku, logros locales,
arte del mando (49 pendiente de aprobación), transición de Opciones, CJK Linux, corrección Linux #19.
Publicados antes: Nightly 4 (trucos, revanchas, aviso moguri, editor de partidas,
#16, diagrama de mando v1). Ver NIGHTLY4_PUBLICACION.md y NIGHTLY5_PUBLICACION.md.

## Esperan reportes (no cuentan como pendientes)

Mando físico, Linux/Steam Deck, Anbernic RG34XX, Android/Odin 2 y móvil real:
se reabren solo si un reporte muestra un fallo.

## Fuentes

LOGICA_APROBADOS.md, PRUEBAS_VISUALES.md, CONTROLADOR_LOGICA.md,
MENU_TRANSICIONES_LOGICA.md, ESCENAS_REVISION_AMPLIA.md, CODEX_TAREAS.md,
CODEX_RESPUESTAS.md, LINUX.md, WEB_PENDIENTES.md. Pruebas e imágenes en build/;
no versionar ROM, saves ni savestates.
