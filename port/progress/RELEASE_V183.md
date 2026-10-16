# v1.8.3 — guante de Naipes mapa

Claude dejó una modificación local sin commitear en src/mode.c. Codex la
recupera, reproduce el fallo y verifica antes de integrar/publicar.

## Causa

El estilo MENU detecta diferencias entre el borde compuesto y su fondo
mientras el menú desliza. Esa rama extendía el píxel visible: cuando el
guante tocaba el borde, su blanco se repetía a lo largo del margen. Es el
mismo mecanismo de los avisos NUEVO de Estado.

PortMenuBackgroundOnly ahora reconoce Mapinspect y Deck además de Status.
Sus márgenes continúan desde BG; el guante, contorno, contador y texto del
rectángulo original se siguen dibujando normalmente. La condición se deriva
del modo vivo y no necesita una bandera adicional para los estados.

## Evidencia

- Reproducción fina: frame219 a 1536x864 en el ejecutable público v1.8.2.
  Franja x0..119, y151..177; desaparece en la versión corregida.
  build/mapcard-transition/glove-before.png y glove-after.png.
- test_card_menu_margins.py --reference build/mapcard-transition/reference:
  12 capturas de secuencias (antes/después, dos menús, tres configuraciones).
  1536x864 x4, 1920x1080 x3, 5120x1440 x6. En la matriz completa el guante
  contaminaba los frames222..225; corregidos en las tres configuraciones.
  Todos los rectángulos originales coinciden píxel a píxel. Frame estable260
  completo conservado en Naipes mapa. El fixture de Deck no muestra nuevos
  cambios. Informe: build/mapcard-transition/review/report.json.
- Estado: test_status_margins.py pasa a 1920x1080 y 5120x1440 con NUEVO on/off.
- EU/US/JP compilan. Mandos físicos aislados solo en los subprocesos de prueba.
- Sin cambios en escalas, x4 predeterminado, audio ni formatos de .sav.

No confundir con el pendiente de texto HD que se superpone al submenú de
Ver barajas: ese problema queda fuera de esta corrección.

## Publicación

Pendiente verificación de ZIP/SHA y preparación local de v1.8.3.
