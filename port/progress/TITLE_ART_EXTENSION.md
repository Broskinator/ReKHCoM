# Ampliacion del arte del titulo — 17/10/2026

Estado: PROPUESTA MANUAL DIBUJADA. No hay arreglo integrado en el render.
Rama feature/title-art-16x9. v1.9 publica permanece intacta.

## Avance manual solicitado por el usuario

El usuario autoriza imaginar y dibujar la continuacion tras los rechazos de
ImageGen. Instalada pixel-art-fundamentals desde ProkStudio/skills mediante
skill-installer, en ~/.codex/skills; consultadas pautas de bordes, valores y
revision. Resolucion, paleta y contexto ya estaban definidos por el usuario
y la inspeccion; no es necesario volver a preguntarlos.

- Receta manual: port/tools/draw_title_extension.py; poligonos/facetas y
  trazos a un pixel nativo, sin IA generativa ni procesamiento de otra API.
- Parche: port/assets/title-study/title-extension-study.png,320x240 RGBA.
  Completa contorno derecho, guante, planos inferiores y puntas del cristal.
  Es una continuacion imaginada, NO arte oficial recuperado.
- Comprobaciones ejecutadas: 38400 pixeles originales identicos, rectangulo
  original transparente en el parche, alpha exclusivamente0/255, todos los
  colores pertenecen a la referencia original.2373 pixeles nuevos.
- Previews privados: build/title-widescreen-review/study/. Vista completa
  title-extension-preview-4x.png y composicion16:9
  title-extension-16x9-mockup.png. Esta ultima es una MAQUETA ESTATICA sobre
  una captura real; NO demuestra que el juego cargue el parche.
- Script de captura ejecuto mode_title, entrada/menu,1536x864 sin saves del
  usuario. No se cambiaron render, menus, zoom, configuraciones ni release.

Pendiente: pulir/revisar arte y costuras antes de integrar; indexar paleta
viva; validar intro/fades/brillo y ratios/contexto de salida. No publicar
como funcional una propuesta que todavia no carga el motor.

## Peticion del usuario

Completar el cristal de Donald y el resto de la zona cyan donde corresponda,
conservar el pixel art original y su tamano; fondo blanco en zonas sin arte.
Completar tambien abajo/derecha para otros encuadres futuros. El usuario pide
una referencia IA en alta resolucion y despues dibujar los pixeles nuevos.

## Inventario comprobado

- mode_title_0 carga gTitleSoraBgTiles y gTitleSoraBgMap en BG1. La imagen
  util es240x160 dentro de un mapa256x256; las celdas restantes son vacias.
- Cyan en la inspeccion representa indice transparente, NO color de fondo.
- gTitleSoraIntroBgMap comparte tiles, pero dibuja Sora sin cristales. No
  contiene la parte perdida. No afirmar que la ilustracion completa esta en ROM.
- Assets EU comprimidos LZ77; US/JP sin comprimir; sprites_title.yaml tiene
  direcciones/tamanos por region. No hardcodear offsets EU en el render.
- Inspeccion privada en build/title-widescreen-review (no subir ROM, mapas
  extraidos, paletas ni referencias descargadas). Imagen del launcher y otros
  originales aportados por el usuario tambien estan recortados a la derecha.
- La imagen del titulo se presenta a altura de menu, no con el zoom del mundo.
  PortSideStyle(PORT_SIDE_COLOR) rellena margenes. No cambiar escala ni mover
  botones para conseguir la ampliacion. Preservar el nucleo240x160 exacto.

## Bloqueo de generacion

ImageGen rechaza en la etapa de salida con moderation_blocked/other, sin
motivo concreto. No hay resultado util. Dos intentos, el segundo solicitado
explicitamente por el usuario como referencia de alta resolucion:
584af057-fc0b-47b0-ab24-8ab51fd428e2 (edicion pixel art)
786e5e36-85d6-4d8c-967a-66bc5013c50b (referencia completa HD)
No reintentar reformulaciones para eludir el rechazo ni presentar una imagen
inexistente como trabajo terminado. No usar otro modelo/API para esquivarlo.

## Continuacion

Hace falta una referencia completa util o una extension dibujada manualmente.
Mantener transparente todo el rectangulo original del parche; añadir solo
pixeles externos. Preferir indices de la paleta viva del titulo para conservar
fades y brillo, sin texturas pegadas sobre el menu. Validar Sora y Riku,
intro/menu,240x160 inalterado,16:9/21:9/32:9/4:3/3:2, cambiar resolucion y
restaurar contexto al salir del titulo. No reflejar ni estirar el cristal.
No integrar un boceto incompleto en stable.
