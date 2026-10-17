# Riku: correcciones locales v1.9 — 17/10/2026

No publicar release: v1.8.3 sigue siendo la unica estable publica.
Trabajo en release/v1.9, sin integrar la escala experimental de menus.

## Persistencia y partidas existentes

- Causa del reset: el motor guardaba cheat_unlock_riku, pero el launcher solo
  serializa kSettings. Al lanzar de nuevo descartaba la clave desconocida.
  Anadida a kSettings con default 0. El submenu sigue OFF en instalaciones nuevas.
- Causa de partidas ocultas: Mode_MenuLoad_0 usaba exclusivamente el bit 5
  (Sora completado). Ahora muestra Riku por acceso opcional O partida pequena
  valida, sin escribir banderas de historia. La pantalla SIO conserva solo Sora.
- Una SRAM mantiene sus 4 ranuras originales: Sora1/Sora2 y Riku1/Riku2.
  No se crean archivos alternativos, ni se convierte formato o region.
- El selector del titulo se actualiza al cerrar Trucos: no requiere reiniciar.
  Conserva Continuar y no expone Link Battle por activar el truco.
- Snapshot privado en build/riku-user-review/20261017-092407. No subirlo.
  Dos Sora y Riku1 validos, sin Sora completado; Riku2 vacio.
- Launcher real: ON y OFF sobreviven 2 aperturas/escrituras cada uno (4 PASS).
  Carga real del snapshot con ON y OFF: ranura 2 -> flags264, Mode_MapFld,
  SRAM completa identica antes/despues (2 PASS).
- Guardado nativo, usando la copia y el menu del punto de guardado: Riku1
  se actualiza; Sora1/Sora2 (ambas copias) y Riku2 permanecen byte por byte.
  Sigue una sola SRAM y la cabecera no marca Sora completado.
  Ruta: titulo A900, cargar A1100, DOWN1400/DOWN1450/A1550,
  fixture mapsave1900, LEFT2000 (Si), A2050. UP no selecciona Si.
- Reproducible: test_riku_persistence.py --save RUTA_A_COPIA --write-roundtrip.
  Fixture esperado: EU, Sora no completado, primera ranura Riku valida.
  Nunca pasar el save que una instancia del jugador este escribiendo.

## Fondo de la introduccion

- Reproducir mediante KHCOM_MODE=10:rikuopening. Usa el enum
  EVENT_149_RIKU_B12F_OPNING, NO el literal 149 (EU omite dos eventos anteriores).
- El fondo tiene 4 bloques: primero contiene el circulo, adyacente otra textura,
  inferiores vacios. Extender el mapa entero exponia otro circulo y negro abajo.
- Contexto consultado por frame desde gEventState, sin nuevo estado serializado.
  Solo esta escena limita BG3 al primer panel. Arcos exteriores reconstruidos
  por fases de tramado 4x4 e indices de paleta nativos; sin RGB interpolado,
  textura repetida, LUT externa ni recursos del juego incluidos en el repositorio.
- El area original se mantiene exacta. Exterior usa paleta viva y compositor
  normal: cambios de paleta y fades no quedan pegados a un color sintetico.
- Tests sin ROM: test_riku_backdrop.py (38400 pixels originales exactos,
  147456 colores de paleta, un centro, cambio de paleta y fade PASS).
  PPU 206/206; padding810 y EXP768 sin fallos.
- Escena real EU: frames300/600 a1536x864, x4: crop GBA exacto vs antes,
  sin negro en borde inferior. PNG local build/riku-backdrop-pixel-perfect.png
  (512x256 indexada, vista de la reconstruccion; no distribuir el arte extraido).
  build/riku-opening-fixed-preview.png muestra el juego con el arreglo.
- EU/US/JP PC compilan. Nuevos APK/Web/Linux aun no generados/publicados.

## Pendientes que siguen abiertos

- Otros cheats/editor de cartas por region/banco/ranura; menus relativos fuera
  de stable; zoom x3 reportado (INI local sigue zoom8=x4, sin reproduccion).
- La prueba de acelerar spans de padding fue descartada: los benchmarks
  pending-pc-before/after-padding se hicieron con carga concurrente y NO prueban
  una mejora incorporada. El renderer general no cambia por ese experimento.
- Recompilar y validar las otras plataformas despues de cerrar los pendientes PC.
