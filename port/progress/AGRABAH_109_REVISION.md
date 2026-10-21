# Agrabah 109: conversación con el Genio, aprobada con honores

21/10/2026. Siguiente fondo distinto después de las escenas 107/108,
aprobadas con honores y congeladas en `2231a8f9f`. Segunda versión aprobada
**con honores** por el usuario el 21/10. Codex prepara el commit; Claude integra.

## Fondo y receta

`gEventBg109` aparece solamente en el evento 109 (`event_index_data.c`).
Mapa 32x32, capas BG3/BG2, tiles/paleta compartidos con Agrabah. Los puestos
y barriles originales se conservan, junto con ventanas y jarrones. La revisión
pedida por el usuario añade decoración estática fuera del mapa (ver abajo).

El pie diagonal de pared mantiene `2*y+x`. La tira donante x=24..31 no lleva
ventanas ni jarrones. Fuera de su rango vertical, BG2 queda transparente y
BG3 toma pared lisa (x=8..11,y=0..3) o arena (x=24..31,y=24..31). Se mantienen
los índices y la paleta activa, sin dibujar tiles nuevos ni hornear capturas.
Solo afecta a píxeles exteriores a los 240x160 GBA y fuera del mapa original.

Identificación por contenido: firmas de las 1024 entradas BG3 `c704bad0`,
BG2 `e1940c58`, idénticas en EU/US/JP; dimensiones 1x1 bloques. Sin depender
del modo ni del número de evento. Se reconstruye al cargar estados o cambiar
de fondo. Activación optativa `KHCOM_AGRABAH109_WIDE=1`.

## Comprobaciones y entrega

`test_agrabah109_backdrop.py` ejecuta en serie, un proceso cada vez. Estados
compartidos antes/después; misma imagen ejecutable; x3 entero en cuatro
ventanas, vista Original regional y escena 107 como control con su extensión
aprobada activada en ambos lados. Evidencia privada bajo
`build/codex-evidence/agrabah-109/`. Las comprobaciones no certifican una
partida completa ni toda la conversación y sus transiciones.

Copia jugable: `../ReKHCoM-prueba-agrabah-109`; lanzador
`../Probar Agrabah 109.bat`. Capturas y ROM privadas, fuera de Git.

## Resultado para revisión

- EU/US/JP compilan. **10/10 casos, 30 pares de imágenes** pasan; cero
  diferencias en los 240x160 GBA. Vista Original idéntica en las tres regiones.
- La extensión aprobada de 107 permanece idéntica en toda la imagen con
  la propuesta de 109 activada/desactivada.
- Primera versión: revisión visual a x3 en los cuatro anchos: pared, base diagonal y arena
  continuadas; puestos, barriles, ventanas y vasijas sin duplicación.
- Informe: `build/codex-evidence/agrabah-109/report-eu-us-jp.json`.
- `propuesta.png` y `antes_despues.png` muestran la captura nativa a x3,
  sin el aviso del cargador de estados. La copia tiene el mismo ejecutable
  que se comprobó. No quedan pruebas ejecutándose.
- Segunda versión aprobada con honores; lista para commit e integración por Claude.

## Segunda propuesta: unión completa y mercado intercalado

El usuario señaló una discontinuidad en la base del muro y pidió llenar el
borde con tiendas y jarrones intercalados para que se vea natural. Esta
petición permite repetir esas piezas decorativas en esta escena; no autoriza
duplicar personajes ni cambia las reglas del resto de fondos.

- El donante x=24..31 está cortado por el borde superior del mapa. Cuando
  cae fuera, se avanza (-8,+4) hasta la tira intacta x=16..23. Así el borde
  continúa por su diagonal y deja de desaparecer al entrar en una columna
  sin dibujo. No se tapa la unión con objetos para ocultar el fallo.
- Puestos originales de fruta y barriles (`gMapGmkAgrabah00/09Tiles`): piezas
  completas de 64x64, con sus anclajes (-32,-48). Se distribuyen con grupos
  de vasijas originales y separaciones distintas. Los puestos originales
  siguen donde estaban. Las piezas nuevas quedan fuera del mapa de 256x256.
- Las vasijas copian el grupo completo de BG2, x=8..39,y=80..111, desplazado
  por la diagonal, incluyendo su contacto con el suelo. Sin mitades de jarra.
- Decoración dibujada en el fondo, fuera de los 240x160, sin crear objetos de
  juego, modificar listas OAM, colisiones, cámara ni personajes. Los puestos
  leen la paleta OBJ activa de sus recursos; las vasijas, la paleta BG activa.
  No se añade una textura externa ni se hornean colores.
- La primera imagen queda recuperable en `propuesta-v1.png` y el primer
  informe en `report-v1.json`. La segunda propuesta fue aprobada con honores.
- Segunda versión: EU/US/JP compilan, 10/10 casos y 30 pares de imágenes
  pasan; GBA y Original idénticos, 107 aprobada sin cambios. Workflow mejorado
  para Preliminar V1 con todas las escenas y jefes, a petición del usuario.
