# Para Claude: bloques listos para el próximo release

Pedido explícito del usuario: ir entregando lo ya terminado mientras se completa
el arte del mando. Todo lo siguiente está en origin/main; integrar con merge/rebase
normal, no duplicar mediante cherry-pick si los commits ya son ancestros.

## Listo

- `3542f817d`: mando en Controles del juego (debajo de bindings), nombres Xbox,
  resaltado por acción/eje y registro del informe físico 8BitDo Ultimate 2C por
  dongle. El dibujo manual es provisional: el usuario acaba de aportar su arte
  Gemini, cuya integración continúa a cargo de Codex.
- `75a7efa60`: arreglo del salto de escala al abrir/cerrar Opciones, fundido
  completo incluyendo HUD/texto HD y luz original del selector debug, sin cambiar
  medidas de cuadros/texto. Solo cambia el lienzo cuando ya está negro.
- Previos `5b39ae4cf` y `a7e63fc78`: logros locales propios EU/US/JP y avisos
  animados arriba centrados; cuenta/catálogo oficial experimental. La evaluación y
  el envío oficial siguen desactivados hasta validar memoria/set. No anunciar
  soporte oficial completo de RetroAchievements.

## Comprobaciones

- Windows EU/US/JP compilan; menú de las tres regiones renderizado.
- `test_zoom.py`: 56 casos pasan tras el arreglo de transición.
- Entrada/salida capturada a 1080p desde título, idiomas y campo. Vídeos privados:
  `build/menu-transition-review/title/menu.mp4` y
  `build/menu-transition-review/controls-motion/menu.mp4` en khcom-codex.
- Informe físico real, SDL virtual=0: 15/15 botones pulsados/liberados, 8/8
  direcciones, cuatro ejes recentrados, LT/RT separados y simultáneos. No incluye
  reconexión/persistencia físicas ni permite afirmar pruebas de otros modelos.

## Aún en curso de Codex

El usuario eligió `Gemini_Generated_Image_lukhx5lukhx5lukh.jpg` de la raíz como
arte del mando. Se está preparando transparencia y máscara de zonas interactivas.
Llega en commit posterior; no dar por aprobado visualmente el dibujo manual.
No modificar este bloque en paralelo mientras Codex lo integra.

Lógica detallada: MENU_TRANSICIONES_LOGICA.md y CONTROLADOR_LOGICA.md.
Cola única: PENDIENTES_VIGENTES.md, puntos 49 y 50.
