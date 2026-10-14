# HUD HD experimental

Solicitud: reconstruir los gráficos del HUD para acompañar al texto HD.
Mantener siluetas, colores, anclajes y animaciones. Opción apagada
por defecto en Experimentales del juego y launcher. Primer piloto integrado:
`hd_hud=1` sustituye el panel enemigo en vistas con capa HUD independiente.
Vista Original conserva el panel GBA. Los demás elementos no se sustituyen.
Sus límites se toman de los sprites originales cada frame, sin ancho fijo.
Marco y letras HP/VT/LP/PV usan imágenes transparentes con relieve/sombra.
Sus caras se ajustan a las cajas de tinta originales. El relleno interpola
luces/cuerpo/sombra de la paleta nativa, con la misma forma, anchura y recorte.
Los valores de naipes usan contornos vectoriales KHMenu en su caja de tinta,
incluyendo transformaciones y oclusión. El resto se ampliará por piezas.

## Formato y rendimiento

Entrada de edición: PNG RGBA con alfa real, resolución inicial 4x y margen
exterior transparente dentro del tamaño previsto. Texto y números siguen
en el renderizador HD existente; no hornearlos en los marcos.

Distribución: atlas PNG sin pérdida y JSON con rectángulos y tamaños lógicos.
PNG se descomprime una vez al cargar el paquete y se sube a una textura SDL.
Liberar los píxeles CPU después de subirla. No decodificar ni generar por
fotograma. WebP reduce potencialmente la descarga, pero no la memoria de
la textura decodificada; no añadir otro decodificador en esta fase.
Presupuesto inicial: un atlas de hasta 2048x2048 (16 MiB RGBA sin mipmaps).
El piloto carga una pieza individual, recortada al subirla y limitada a
512 píxeles de ancho: unos 320 KiB entre textura y máscara de fade blanco.

## Preparación

`python port/tools/pack_hd_hud.py manifest.json build/hud-pack`

Ejemplo de manifiesto (rutas relativas a este archivo):

```json
{"scale":4,"pieces":[{"id":"hp_frame","file":"hp.png","width":64,"height":16}]}
```

Las dimensiones del ejemplo son ilustrativas; medir cada pieza original.
La herramienta rechaza PNG sin RGBA, imágenes vacías, tamaños incorrectos,
IDs duplicados, bordes exteriores opacos y paquetes sobre presupuesto.
Produce vistas sobre fondo claro y oscuro para revisión de halos y fondos
dibujados: el alfa por sí solo no demuestra que toda la imagen esté limpia.
No incorporar piezas con tablero falso, fondo residual o contorno alterado.

## Pendiente de integración

- Extraer las próximas piezas originales con alfa y referencias de paleta/estado.
- Piloto generado con imagegen y segunda pasada para diagonales suaves:
  `port/assets/hd-hud/pilot/enemy-hp-frame-v2.png`. Prompts en su README.
- Cargador PNG del juego implementado en `port_hdhud.c`, límites de tamaño,
  alfa exterior obligatorio y CPU liberada después de crear la textura.
- Identificación del panel por su tarea de juego, sin direcciones VRAM.
- Vida y capas calculadas por el juego; letras y marco nuevos con fade negro
  y blanco. Ampliar a otras piezas y revisar oclusión/animaciones complejas.
- Retorno al HUD original si falta/falla el recurso o se apaga la opción.
- `test_hd_hud.py`: capturas comparadas con opción apagada, imagen ausente
  y vista Original, y coincidencia exacta de los píxeles verdes de la barra
  del relleno (posición y tamaño) en los frames 180 y 380 del combate
  piloto. No representa una auditoría de todos los jefes ni de todas las
  paletas/idiomas. Medición de rendimiento sostenido aún pendiente.

Herramienta candidata de edición: GPT Image, PNG transparente
([guía oficial](https://developers.openai.com/cookbook/examples/multimodal/image-gen-models-prompting-guide)).
Alternativa local: FLUX Kontext
([proyecto oficial](https://github.com/black-forest-labs/flux/blob/main/docs/image-editing.md)).
Ninguna garantiza automáticamente fidelidad de forma ni transparencia perfecta.
