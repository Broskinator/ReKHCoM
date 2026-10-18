# Claude — cierre de su parte de la repartija (18/10/2026)

Rama `claude/saves` (checkout khcom-claude-int), sobre `c201169a9` de main.

## Hecho

- **Guardados, puntos 5–9: `e9f053595`.** Al cambiar de ranura se muestra lo que
  contiene la ranura (vacía si no hay partida), como la ventana original: decisión
  del usuario. Guardar en la otra ranura la convierte en la partida actual: se
  escribe solo su resumen y su cabecera, los autoguardados posteriores van ahí y
  el cristal vuelve a abrirse en ella (la marca va dentro de la partida). El
  launcher ya no borra `experimental_save_point_slot_select` de kh_com.ini,
  conserva las claves que no conoce y muestra la opción en Experimental.
  Prueba: harness `room:20000` + `mapsave` (EU): guardar en la 1, cambiar,
  guardar en la 2, reabrir. La 1 conserva 00:00:11, la 2 muestra 00:00:22 y el
  cristal reabre en la 2. EU/US/JP compilan.
- Issue #12 actualizado con estos arreglos (pendientes de publicar).
- **Codex:** los guardados no son tuyos. Descarta tu copia sin commitear de
  `src/map/map.c` y `port/decompui/decompui.cpp` en khcom-codex.

## Savestates (respaldados en build/userstates/nightly2 con su ejecutable)

- slot1 (04:27): pasillo de Hollow Bastion con Riku; la pared izquierda queda
  con el borde superior recto en vista amplia. Issue #17.
- slot3 (04:34): Mapamundi de Hollow Bastion; damero de la banda inferior.
  Al cargarlo a 16:9 y 32:9 la banda sale completa; falta confirmar con el
  usuario la zona exacta. Issue #18.
