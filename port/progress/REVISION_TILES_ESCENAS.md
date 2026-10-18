# Revisión acumulativa de escenas con extensión de tiles

Estado 18/10/2026: **3/44 candidatas aceptadas**. Son 44 eventos con cambios
potenciales en 33 fondos; los vacíos que forman un marco intencionado se
descartan en revisión. No interpretar 44 como 44 fondos que haya que rellenar.

| Evento | Estado | Ejemplo / regla aprendida |
| --- | --- | --- |
| 004 Leon | Aceptada previamente | Continuar diagonales y período propio, sin duplicar cajas. |
| 061 Cristal | Mejorada aceptada | Perfiles distintos por capa; rombo entero en la unión; seleccionar tile de panel sin sombra ajena. |
| 057 Playa | Siguiente en revisión | Período del suelo 8×4; pared con huecos requiere revisar capas por separado. |
| 074 Monstruo | Mejorada aceptada | Paleta activa y tres capas; pieza inferior completada con el motivo izquierdo original. |

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
9. Distinguir tamaño declarado y bloques realmente dibujados. Un mapa puede
   declarar 512×512, pero contener un solo bloque de escenario de 256×256 y tres
   bloques predeterminados vacíos (074). El borde útil no siempre es el borde
   exterior de la declaración. Detectar esto antes de buscar períodos.
10. No omitir capas superiores solo porque parecen decorativas: en 074, BG1
    cubre tiles que no están diseñados para verse directamente. Extender las tres
    capas y revisar si aparecen adornos repetidos o cortados antes de aceptar.
11. Usar la paleta activa del fotograma, con la misma conversión de 5 a 8 bits
    del PPU. En 074 el juego aclara las entradas de la paleta; dibujar el añadido
    con la paleta ROM dejaba el fondo negro y tonos más saturados. No era un
    fallo del layout. `scene_tile_data.active_bg_palette` guarda esta lógica
    compartida. Para shot100 de esta captura se usa el snapshot99: el snapshot100
    ya contiene la siguiente actualización de la paleta. BLDCNT/BLDALPHA/BLDY
    están en cero, por lo que aquí el efecto se debe a la paleta, no a blending.
    En el motor usar siempre PLTT activa; no fijar un aclarado constante de 49 RGB.
12. Antes de extender un adorno cortado con el período general, buscar una copia
    completa del mismo adorno en el mapa original. En la mejora de 074, 19 entradas
    ya visibles de BG2 coinciden con la pieza izquierda desplazada (+20,-1) tiles.
    Esto permite completar el extremo derecho con coordenadas fuente (x-20,y+1),
    sin inventar un nuevo fragmento ni volver a copiar el comienzo del adorno.

Mejora 074 aprobada por el usuario: `py -3 port/tools/scene074_polish_preview.py`.
El commit preliminar es `8cf93bc78`; permanece congelado. Solo BG2 destino
x=32..34, y=17..21 toma entradas fuente (x-20,y+1). La comparación muestra
la pieza completa y elimina el fragmento repetido inferior; 507 píxeles cambian,
todos dentro de la franja añadida. El header de integración contiene esta
sustitución; la base preliminar sigue disponible en su commit.

Propuesta 074: `py -3 port/tools/scene074_tile_preview.py`; encuadre frame100,
cámara (0,0), perfil de derecha/abajo. Salida privada `review074/074-before-after.png`.
En este encuadre solo el borde derecho sale del bloque activo; abajo no se cruza
todavía, así que este primer comparativo no valida visualmente el borde inferior.
Base preliminar aceptada, congelada antes del ensayo del detalle verde en
`build/scene-review/accepted074/`. Perfil para main:
`port/include/port/port_scene074_tiles.h` (aún sin conectar aquí al PPU).
Usar la paleta/effectos del motor y llamar dentro de los bloques default también.

Por revisión: mostrar el mismo encuadre antes/después; conservar cámara, capas,
bordes, pasos y sustituciones. Actualizar el contador al aceptar o descartar una
escena. Los perfiles concretos aprobados son ejemplos; no aplicar sus números
indiscriminadamente a otros mapas.
