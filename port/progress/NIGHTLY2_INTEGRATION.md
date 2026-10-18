# Nightly 2: Guard Armor y perfiles aceptados de escenas

18/10/2026. Versión `v1.9.3-nightly.20261018.2`; pre-release Windows, sin
promoción a estable. El usuario acepta Guard Armor y pide integrar los commits
de fondos del otro chat antes de continuar con Darkside.

## Integración

- Guard Armor: `aa2227330`, patio extendido con tiles/píxeles del cartucho y
  paleta activa. Criterio y técnica en `BATTLE_SCENERY_TECHNIQUE.md`.
- Commits originales revisados e incorporados del otro chat:
  `a3ff671f6`, `18ef1948f`, `8cf93bc78`, `a4427b269`, `c924ff625` y
  **`b651e4b65`** (playa mejorada aprobada, pasto completo).
- Esos commits guardaban perfiles y previews; se añadió su conexión real al
  PPU para 057 (BG3/BG2), 061 (BG3/BG2) y 074 (BG3/BG2/BG1).
- La selección de perfil se deriva del evento activo en cada frame. No se
  añade una bandera de escena que sobreviva a otra escena/carga de estado.
- El perfil se aplica antes de los límites declarados del mapa: en 074 el
  mapa 64×64 solo tiene un bloque 32×32 ocupado. Se conservan los vacíos
  intencionados, los sprites, la ventana GBA y las paletas/efectos hardware.
- El trabajo sin commitear del otro worktree no se modifica ni se integra.

## Evidencia

- Windows EU/US/JP: build correcto.
- `test_guard_armor_backdrop.py`: **13/13**, vuelto a ejecutar sobre el motor
  integrado. Compara antes/después del mismo binario, tres frames/caso.
  Cero cambios en el rectángulo GBA; Original EU/US/JP, Hook y Monstro
  permanecen idénticos. Sora EU/US/JP y Riku EU, 16:9/32:9.
- `test_accepted_scene_profiles.py`: **23/23**, frames 100/180/300. Perfiles
  057/061/074 en EU/US/JP a 16:9 y 32:9; Original EU sin diferencias; evento
  004 de control sin diferencias. Cero cambios en el rectángulo GBA.
- Capturas privadas: `build/codex-evidence/accepted-scenes-integrated/` y
  `build/codex-evidence/guard-patio-integrated-tests/`.
- La preflight inicial de escenas no iniciaba el evento (faltaba A en el
  menú debug). Se corrigió el harness y se repitió; los 23/23 corresponden al
  evento realmente ejecutándose, no al menú. No contar la preflight como PASS.

No es una validación de partidas completas, todos los zooms o todas las fases.
Las pruebas de guardado real de #12 siguen pendientes y la opción queda Off
por defecto bajo Experimental. RetroAchievements sigue pendiente de integrar.

## Contadores y continuidad

- Escenarios de jefes: **1/8 aceptado e integrado**, Guard Armor.
- Escenas: **4/44 aceptadas** (004, 057, 061, 074); no confundir con 44 fondos
  que haya que rellenar. Se continúan las partes aprobadas, no todos los huecos.
- Siguiente jefe: **Darkside**, separar el fondo playa/cielo de la capa del
  jefe para evitar los recortes y duplicaciones observados.
