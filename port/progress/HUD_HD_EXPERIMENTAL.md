# HUD HD experimental

Solicitud: reconstruir los gráficos del HUD para acompañar al texto HD.
Mantener siluetas, colores, anclajes y animaciones. Opción futura apagada
por defecto en Experimentales del juego y launcher. No es aún una función
disponible: esta primera entrega prepara el formato de recursos.

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

- Extraer piezas originales con alfa y referencias de paleta/estado.
- Redibujar una pieza piloto con IA y revisar forma y bordes sobre ambos fondos.
- Cargador PNG del juego: el launcher usa stb_image, el juego aún no tiene
  un cargador de reemplazos HUD. Reutilizar PNG solamente y limitar tamaños.
- Identificar piezas por contenido/estado, no por dirección VRAM cambiante.
- Sustituirlas respetando oclusión, fade, flips y animaciones antes del texto HD.
- Mantener gráficos originales si el pack falta, falla o la opción está apagada.
- Validar EU/US/JP y medir tiempos de frame y memoria con la opción activa.

Herramienta candidata de edición: GPT Image, PNG transparente
([guía oficial](https://developers.openai.com/cookbook/examples/multimodal/image-gen-models-prompting-guide)).
Alternativa local: FLUX Kontext
([proyecto oficial](https://github.com/black-forest-labs/flux/blob/main/docs/image-editing.md)).
Ninguna garantiza automáticamente fidelidad de forma ni transparencia perfecta.
