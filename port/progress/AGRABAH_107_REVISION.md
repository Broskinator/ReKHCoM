# Agrabah: calle de Aladdin (107/108), aprobada con honores

21/10/2026. El usuario aprobó la captura final y confirmó **«aprobada con
honores»**. Codex prepara el commit; Claude integra. La activación sigue
siendo optativa en esta rama para que Claude controle la integración.

## Receta

Los eventos 107 y 108 cargan `gEventBg107` (`event_index_data.c`). La arena
del encuentro usa otro fondo; Jafar y los restantes fondos de Agrabah quedan
fuera de esta propuesta.

`port_agrabah_tiles.h` continúa las capas BG3 y BG2 solo fuera de su mapa
de 64x64 tiles y solo en los píxeles exteriores a los 240x160 de GBA. Las
tiras laterales de cuatro tiles no contienen ventanas ni esquinas únicas.
La calle conserva la diagonal `2*y+x`; las fachadas de azoteas conservan
`2*y-x`. El techo abierto entre ambos bordes usa su textura lisa original,
sin volver a muestrear las otras fachadas. Arriba y abajo se usan bandas
lisas del propio mapa. No se dibujan nuevos tiles, colores ni objetos.

El renderizador identifica los mapas por sus dimensiones y la firma FNV de
sus 4096 entradas, idéntica en EU/US/JP. EU los descomprime en RAM; US/JP
los usan directamente. Se comprueba el contenido cada frame para evitar
identificar una asignación de RAM reutilizada o un estado anterior. No exige
un modo ni un número de evento. Los tiles siguen usando la paleta activa.

Prueba optativa: `KHCOM_AGRABAH_WIDE=1`. Sin ella, comportamiento previo.
No cambia colisiones, cámara ni lógica del juego.

## Revisión y entrega

- Compilaciones aisladas: `build/agrabah-eu`, `build/agrabah-us`, `build/agrabah-jp`.
- Harness: `python port/tools/test_agrabah_backdrop.py`.
- Evidencia privada: `build/codex-evidence/agrabah-107/report.json` y capturas.
- Misma instantánea de juego y ejecutable en ambos lados de cada comparación:
  evita divergencias de avance de texto/animación entre arranques independientes.
- Cinco estados por caso, con entrada/fundido y cámara inicial/desplazada;
  cuatro ventanas x3 (1920x1080, 2560x1080, 3440x1440 y 3840x1080).
- Controles: vista Original EU/US/JP y evento 109 (otro fondo).
- La escena 108 japonesa no se certifica: el productor de estados sin la
  extensión se bloqueó tras guardar el primer estado; se detuvo esa ejecución.
  La escena 107 japonesa sí entra en las comparaciones. No se modificó la
  lógica del evento para sortear ese bloqueo.
- El productor US de 108 mostró otra escena (Riku) en vez de Agrabah; se
  conserva como intento inválido y no cuenta como verificación de 108. Se
  verifica 107 en US. Una primera comparación Original JP difirió en un
  estado; su repetición aislada dio cinco estados idénticos. Se conserva el
  informe inicial y se repite el conjunto final para registrar el resultado.
- Copia local: `../ReKHCoM-prueba-agrabah-107`, lanzador `../Probar Agrabah 107.bat`.

No se incluyen ROMs, instantáneas ni imágenes extraídas en Git. Las muestras
no certifican una partida completa, todas las transiciones ni todos los zooms.

## Estado al liberar recursos para Deadlock

El conjunto anterior al último ajuste de una fila del bordillo pasó 11/11
comparaciones (cinco estados por caso). La versión de entrega ya incluye ese
ajuste (`2*y+x >= 118`, `2*y-x >= 7`) y compila en EU/US/JP; se renovaron
los productores de estados para la comprobación final, pero queda pendiente
repetir las comparaciones de esa última modificación antes del commit.
Las capturas del paquete son anteriores a ese ajuste de una fila.
No quedan procesos de prueba activos. El usuario pidió cuidar RAM mientras
juega Deadlock; no lanzar pruebas paralelas. Copia jugable actualizada con
el último ejecutable EU. Después se capturó `propuesta-final.png` desde el
ejecutable actualizado, y esa imagen recibió la aprobación con honores.

## Cierre posterior a la aprobación

Último ejecutable: EU/US/JP compilan. Comparación final en serie, sin juegos
simultáneos: **11/11 casos, 55 pares de imágenes**, región GBA idéntica en
todos. Cuatro anchos x3 EU, eventos 107/108 EU, 107 US/JP, Original EU/US/JP
y otro fondo (109) sin cambios. Informe: `report-final.json`. Los límites
regionales de 108 señalados arriba se mantienen; no cuentan como aprobados.
