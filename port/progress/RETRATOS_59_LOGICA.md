# 59: retratos adelantados fuera de su ventana

## Causa y corrección

El juego crea el retrato antes de abrir el diálogo. Sus coordenadas de espera
son X=449 y X=-299 (gMsgfaceHiddenX, punto fijo 8.8). La paleta inicial es la del
personaje; la orden de entrada aplica después la silueta cuando corresponde.
La GBA no muestra la posición de espera, pero el margen panorámico sí la exponía.
Esto reproducía a DiZ a color y la cara suelta de Riku en EVENT_149.

- msgface_2 omite el dibujo en la coordenada de espera, sin modificar actualizaciones,
  paletas, comandos, guion ni banderas de historia.
- PortSpriteClipX etiqueta únicamente las piezas de ese retrato con los límites
  originales de su ventana, trasladados por gPortSpriteShiftX. El PPU respeta esos
  límites al rasterizar, también en affine. Así la entrada/salida no muestra las
  piezas que la GBA ocultaba. El alcance se cierra antes de dibujar otros objetos.
- Todo está bajo PLATFORM_PC. No cambia el núcleo original de GBA ni el contenido
  de sprites/ROM. No se condiciona a HD text: la cara es un sprite, no texto HD.
- KHCOM_PORTRAIT_UNCLIPPED=1 es la comparación del harness con el comportamiento
  anterior, no una opción de usuario.

## Verificación

test_portrait_clipping.py: 12 comparaciones, 24 procesos nativos aislados: EU/US/JP,
Sora (EVENT_004, León) y Riku (EVENT_149, apertura), GBA y vista amplia. Los 48 pares
de capturas GBA 240x160 son idénticos píxel a píxel. La comparación es con la vista
original del port y la ruta previa, no con una grabación independiente de emulador.
Se conserva el diálogo mostrado y desaparecen los retratos aparcados. Builds de
las tres regiones correctos. No equivale a una partida completa de todas las escenas.

Evidencia privada: build/portrait59-regression/report.json y capturas. Reproducción
exacta marcada por el usuario: build/portrait59-output/{before,after}/out_01200.png,
1920x1080, zoom x3. También out_00900.png para la cara de Riku.

## Entrega visual

Copia separada en la raíz ReKHCoM-prueba-retratos-59, con datos propios. BAT:
Probar retratos 59 - Riku.bat y Probar retratos 59 - Sora.bat. El preset solo
selecciona la escena; el usuario avanza el texto normalmente. No se cargan ni
reescriben sus partidas. Propuesta lista para revisión visual; no declarada aprobada.
