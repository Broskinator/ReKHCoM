# Revisión acumulativa de escenas con extensión de tiles

Estado 18/10/2026: **4/44 candidatas aceptadas**. Son 44 eventos con cambios
potenciales en 33 fondos; los vacíos que forman un marco intencionado se
descartan en revisión. No interpretar 44 como 44 fondos que haya que rellenar.

| Evento | Estado | Ejemplo / regla aprendida |
| --- | --- | --- |
| 004 Leon | Aceptada previamente | Continuar diagonales y período propio, sin duplicar cajas. |
| 061 Cristal | Mejorada aceptada | Perfiles distintos por capa; rombo entero en la unión; seleccionar tile de panel sin sombra ajena. |
| 057 Playa | Mejorada aprobada | Pared continua; pasto completo a ambos lados de la unión con piezas originales. |
| 074 Monstruo | Mejorada aceptada | Paleta activa y tres capas; pieza inferior completada con el motivo izquierdo original. |

## Lógica que se acumula con cada ejemplo

Corrección explícita del usuario: no tomar ensayos fallidos o ejemplos feos de
otras propuestas como referencias para escenas nuevas. Arrancar cada propuesta
desde el original del juego y sus piezas; las reglas técnicas documentadas y las
versiones aprobadas se conservan. Retiradas las tarjetas fallidas `cards/057.png`,
`cards/p_057.png` y `cards/t1_057.png`. La nueva receta 057 no lee ningún "after"
histórico, solo ROM, snapshot de paleta y captura original sin extensión.

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
    **Comprobación obligatoria antes de presentar**: aplicada y aprobada en
    074 (adorno verde) y 057 (pasto). Buscar todas las mitades de cada motivo
    que cruce la unión, incluso si las líneas de suelo y pared ya continúan bien.
13. Revisar motivos a ambos lados de la unión, aunque la geometría estructural
    ya encaje. En 057 la extensión izquierda dibuja media mancha de pasto y el
    mapa original contiguo tiene arena; completarla exige continuar la misma
    pieza unos tiles dentro del mapa original. A derecha ocurre el caso inverso:
    la mitad original ya está y falta la mitad del añadido. No resolver ambos
    casos con el período general de madera/arena; usar la copia completa que
    coincida con las partes visibles de cada mancha. Mantener las capas de pared
    encima y evitar modificar tiles estructurales de madera al rellenar suelo.

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

057 nueva: `py -3 port/tools/scene057_tile_preview.py`. Cámara(7,43), frame100,
suelo y pared a izquierda con fuente (+8,-4), siguiendo su base ascendente a la
derecha. Suelo a derecha (-8,+4); BG2 transparente ahí para no copiar el tronco
único de la zona original. Paleta activa del snapshot camera057. Perfil nuevo
aceptado como mejorable, base congelada en `accepted057/`. Salida `review057/057-before-after.png`.

Mejora 057 aprobada: `py -3 port/tools/scene057_polish_preview.py`.
Base aceptada `c924ff625`; se conserva. BG3 izquierda destino x0..4,y18..22
fuente(x+8,y-4), para cerrar la mitad que cruza al lado original. Derecha BG3
x32..35,y5..11 fuente(x-15,y+4): nueve tiles con pasto coinciden exactamente
con el motivo completo del mapa. Cambian1590 píxeles, 600 en el encuadre GBA
original por la continuación del pasto izquierdo. Ningún cambio de pared ni
palmera. Salida privada `polish057/057-polish-before-after.png`.

## Integración en main y Nightly 2 (18/10/2026)

Los perfiles aprobados de 057/061/074 ya están conectados al PPU con paleta
activa, junto a 004 que ya estaba integrada. Incluye el pasto mejorado de
`b651e4b65`. Build EU/US/JP y 23/23 comparaciones de escenas PASS; Original
y región GBA idénticos. Ver `NIGHTLY2_INTEGRATION.md`. Esta nota actualiza
las referencias históricas anteriores que describían los perfiles sin hook.
