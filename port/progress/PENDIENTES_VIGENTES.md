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

## Reparto actual (18/10, noche)

| Quién | Puntos | Orden sugerido |
|---|---|---|
| **Claude (Opus)** | Publicar v1.9.4 estable y responder #19 (en curso); 44 limpieza; 43 mantener la cola; 51 Vita (por fases); 39 rutas completas (al final) | 1) cerrar v1.9.4, 2) 44 con permiso, 3) 39 |
| **ChatGPT/Codex** | 16–24 arte de jefes/escenas; 27 RetroAchievements; 29 Mapamundi/objetos de sala; 36–37 Web en navegador real; 40 audio PSG; 2 cierre del chat sincorazones | 1) 29 y 40 (cortos), 2) 36–37, 3) 27, 4) 16–24 al final |

Reglas de continuidad y orden por tamaño: lo corto primero; arte de jefes y rutas
completas al final.

## 1. Por publicar

- v1.9.4 estable: compilada la fuente; falta Android/Web, reconstruir Linux (ícono) y publicar.

## 2. Codex

- **16–24:** arte de jefes/escenas, al final y de a uno con antes/después:
  16 Darkside (WIP en rama wip/darkside-backdrop-20261018, fuera de main),
  17 Oogie, 18 Jafar, 19 Maléfica dragón, 20 Úrsula, 21 Marluxia final,
  22 Alicia/Trickmaster (tablero inferior), 23 resto de candidatas,
  24 tira de fondo de arenas GBA.
- **26:** CERRADO 18/10: issue #2 (funciones del remake) cerrado por el usuario como
  mod muy específico, fuera de alcance por ahora.
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
- **51 (nuevo, 18/10): PS Vita.** Decisión del usuario: de ggdrn/KH_COM_VITA (MIT, v0.08.08)
  solo se toma la FORMA DE COMPILAR (VitaSDK + vitaGL, ROM sin datos en el .vpk con
  tira/auditoría y carga al arrancar desde ux0:data, kubridge opcional); todo lo demás
  es nuestro (PPU, m4a, opciones, trucos, vista amplia, guardados). Fases:
  1) VitaSDK+vitaGL en la VM Debian, compilar su repo como referencia y probar el .vpk en
  Vita3K (Windows); 2) build Vita de nuestro árbol: punteros reubicables en vez de
  direcciones GBA fijas (la Vita no permite mapa fijo), macros de acceso a hardware,
  capa propia de vídeo/audio/entrada (vitaGL, sceAudioOut, sceCtrl); 3) .vpk alpha
  (EU/US/JP) con aviso de ROM propia; validación en Vita real = reportes. Atribución MIT
  en NOTICE si se adapta algún script. Grande (semanas); va después de lo corto.
- **52–54 (18/10, el usuario ordena empezar PS4, Switch y Vita junto con PS5; orden:
  PS5 estable → Vita fase 1 → PS4 → Switch; estimación sin probar):**
  52 **PS4** (el más viable; el guest x32 de port/ps5 es el mismo código, falta host propio con
  sceVideoOut/sceAudioOut/pad dibujando con nuestro PPU por software; probar en shadPS4):
  x86-64 como la PS5; reutilizar el guest de 32 bits y el
  puente host de port/ps5 con otro SDK (OpenOrbis/GoldHEN) y otra salida de vídeo.
  53 **Switch**: aarch64 de 64 bits; exige o bien la pasada de 64 bits del núcleo o un
  proceso de 32 bits (a verificar en libnx/Horizon). 54 **PS3**: Cell PowerPC big-endian
  (el juego asume little-endian en estructuras y datos de ROM), 256 MB: muy improbable.
  La pasada de 64 bits desbloquearía también macOS (hoy solo Web o Wine). Hardware real = reportes.
- **43:** mantener esta cola y los contadores tras cada cierre.

## Cerrados hoy (referencia)

Mando (49) y 057 playa (12) aprobados por el usuario. Guardados y opciones (5–9), #17 borde de Riku (10), #18 Mapamundi (11), 061 hielo
y 074 Monstro (13–14), remapeo y gatillos (25), submenús (28), fuente CJK Linux
(34), PS5 (38), táctil (45), transición de Opciones (50), regresión sincorazones (4).
Publicado en Nightly 5 (18/10, v1.9.3-nightly.20261018.5): #17 borde de Riku, logros locales,
arte del mando (49 aprobado), transición de Opciones, CJK Linux, corrección Linux #19.
Publicados antes: Nightly 4 (trucos, revanchas, aviso moguri, editor de partidas,
#16, diagrama de mando v1). Ver NIGHTLY4_PUBLICACION.md y NIGHTLY5_PUBLICACION.md.

## Futuro muy lejano (no cuenta como pendiente)

- 057 playa (12, aprobada): igualar los tiles del lado superior derecho con el
  izquierdo y que el pasto no tenga motivos cortados. Solo si algún día sobran tokens.

## Esperan reportes (no cuentan como pendientes)

Mando físico, Linux/Steam Deck, Anbernic RG34XX, Android/Odin 2 y móvil real:
se reabren solo si un reporte muestra un fallo.

## Fuentes

LOGICA_APROBADOS.md, PRUEBAS_VISUALES.md, CONTROLADOR_LOGICA.md,
MENU_TRANSICIONES_LOGICA.md, ESCENAS_REVISION_AMPLIA.md, CODEX_TAREAS.md,
CODEX_RESPUESTAS.md, LINUX.md, WEB_PENDIENTES.md. Pruebas e imágenes en build/;
no versionar ROM, saves ni savestates.
