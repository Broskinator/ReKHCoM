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
  completo conservado en Naipes mapa. El fixture corregido de Deck (Mode_Deck, nombre sensible a mayúsculas)
  no muestra nuevos cambios. Informe: build/mapcard-transition/review/report.json.
- Estado: test_status_margins.py pasa a 1920x1080 y 5120x1440 con NUEVO on/off.
- EU/US/JP compilan. Mandos físicos aislados solo en los subprocesos de prueba.
- Sin cambios en escalas, x4 predeterminado, audio ni formatos de .sav.

No confundir con el pendiente de texto HD que se superpone al submenú de
Ver barajas: ese problema queda fuera de esta corrección.

## Publicación

ZIP verificado: CRC íntegro, cuatro ejecutables idénticos a los builds probados,
ninguna ROM/partida/estado/configuración personal. 21.078.082 bytes, SHA-256:
b3abf8685532393b0f0b4ab090dbc30757cd835b840e5398ecbd7f1dc5f83b51.

Copia local ../ReKHCoM-v1.8.3/ReKHCoM-v1.8.3-windows-x86 y acceso
../Probar ReKHCoM v1.8.3.bat. Se copian las partidas normales y ajustes de
v1.8.2 sin sobrescribir; los estados rápidos antiguos permanecen con su build.

Pedido adicional: única stable v1.8.3, tags anteriores conservados. README con
resumen para jugadores y galería renovada. Captura reproducible:
port/tools/refresh_readme_screenshots.py; manifiesto local y hoja de revisión
en build/readme-v183. Selección de idioma real antes de abrir cada menú;
no enviar confirmaciones A una vez abierto el menú para evitar capturar combate.
La revisión detectó que el primer fixture usaba mode_deck y solo comprobaba
una subcadena presente incluso en el aviso «no mode». Ahora usa Mode_Deck,
rechaza ese aviso y verifica la entrada real. Matriz repetida: PASS.
Las dos pantallas inalteradas de selección de nueva partida siguen de referencia.

Publicada: https://github.com/MatyMeatBoy/ReKHCom/releases/tag/v1.8.3
Release id 414677687; GitHub confirma el digest del ZIP y latest=v1.8.3.
Única release pública estable. La anterior v1.8.2 se retiró después de verificar
la nueva; conserva su tag d7b95a8cd y copia local. Metadatos previos archivados
en build/releases/v1.8.2-release-before-removal.json.
Commit de juego cfc5b6392; tag v1.8.3 incluye la documentación d462f08cc.
