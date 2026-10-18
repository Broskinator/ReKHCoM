# Revisión acumulativa de escenas con extensión de tiles

Estado 18/10/2026: **2/44 candidatas aceptadas**. Son 44 eventos con cambios
potenciales en 33 fondos; los vacíos que forman un marco intencionado se
descartan en revisión. No interpretar 44 como 44 fondos que haya que rellenar.

| Evento | Estado | Ejemplo / regla aprendida |
| --- | --- | --- |
| 004 Leon | Aceptada previamente | Continuar diagonales y período propio, sin duplicar cajas. |
| 061 Cristal | Mejorada aceptada | Perfiles distintos por capa; rombo entero en la unión; seleccionar tile de panel sin sombra ajena. |
| 057 Playa | Pendiente | Período del suelo 8×4; pared con huecos requiere revisar capas por separado. |
| 074 Monstruo | Próxima propuesta | Solo derecha y abajo; conservar vacío diseñado arriba e izquierda. |

## Lógica que se acumula con cada ejemplo

1. Determinar por separado los bordes que se cortan y los vacíos diseñados.
2. Buscar período y dirección en cada capa, guiándose por líneas estructurales
   y ponderando coincidencias por la rareza de cada tile.
3. Fijar los valores aprobados por escena. No volver a cambiar de distribución
   al regenerar imágenes porque otro período puntúe parecido.
4. Resolver las esquinas sin ciclos, reflejando el paso cuando sea necesario.
5. Los motivos grandes requieren continuidad de varias piezas. Completar una
   pieza que cruza el borde puede necesitar cambiar también el lado original;
   mostrar esa modificación expresamente para revisión (ejemplo 061).
6. Una capa superior puede volver a dibujar la pieza anterior: usar transparencia
   original en esa zona cuando corresponda, manteniendo los demás objetos.
7. Para sombras o fragmentos ajenos, buscar otro tile original con los mismos
   píxeles estructurales y sin el detalle no deseado. Guardar coordenadas fuente,
   destino y comparación. No arreglar capturas como si fueran assets del juego.
8. Congelar cada versión aceptada en un commit antes de nuevos ensayos. Guardar
   recetas en Git e imágenes y ROM extraídas solo en build/ privado.

Por revisión: mostrar el mismo encuadre antes/después; conservar cámara, capas,
bordes, pasos y sustituciones. Actualizar el contador al aceptar o descartar una
escena. Los perfiles concretos aprobados son ejemplos; no aplicar sus números
indiscriminadamente a otros mapas.
