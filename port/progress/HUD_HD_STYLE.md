# HUD HD: criterios visuales obligatorios

## Orden de trabajo

Completar primero el lote preliminar de todas las familias e integrarlo en
Experimentales → HUD HD (apagado por defecto). Después comparar y pulir el
conjunto contra el original. Codex deja commits locales; Claude integra y publica.

## Referencia y fidelidad

- La composición aprobada de Sora (`sora-player-assembly-v1.png`) fija la
  calidad de acabado: contornos limpios, metal fino y sombras contenidas.
- El sprite original fija silueta, orientación, perspectiva, proporciones,
  colores, degradados, elementos interiores y posición. Mejorar resolución
  no autoriza reinterpretar su cámara ni inventar piezas.
- Mantener el degradado azul de los naipes, sus extremos oscuros y su dirección.
  No reemplazarlo por un color plano o un brillo genérico.
- La llave se dibuja en el plano del original: mango, guarda y hoja comparten
  perspectiva. El usuario rechazó el mango isométrico unido a una hoja plana.
  No añadir extrusión, escorzo, ángulos, orificios ni adornos ausentes.
- Feedback posterior crítico: la versión aplanada perdió la forma aprobada
  del Keyblade. Usar como base la primera ilustración aprobada, conservar
  todos sus contornos y formas curvas y limitar la corrección de perspectiva
  al sombreado. No transformar cilindros redondeados en bloques angulares.
- Conservar sombras/relieve originales en HD, sin fondos falsos ni halos.
- Marcos y rellenos son independientes. La vida y recarga siguen datos,
  interpolación y capas originales; no se incrustan valores en las imágenes.
- Punta redondeada del relleno según el estado original de vida completa;
  no dejarla como decoración fija al disminuir la vida.

## Rutina por pieza

### Paleta y acabado: feedback vigente

El usuario rechazó las últimas cartas por apariencia genérica de IA. Mantener
el toon y la suavidad del GBA es obligatorio, además de conservar la geometría.
No confundir HD con metal brillante, biseles nuevos o contornos negros gruesos.
Las versiones v4 y v5 siguen como borradores: v4 altera el acabado y v5 tiene
bordes irregulares y volumen excesivo en la guarda. Ninguna se empaqueta.

Paleta medida del original `8e08863da15d6f89`: fondo `#000884`, `#0008B5`,
`#0018D6`; contorno `#213131`; amarillo `#FFFF29`, sombra `#CE9C00`;
acero `#7B848C`, `#B5BDDE`, `#525A63`. Usar estos tonos como anclas,
interpolando suavemente para antialias y degradados; no sustituir por azul
eléctrico ni oro realista. La revisión de estilo es manual y separada de las
comprobaciones técnicas de alfa, memoria y carga.

1. Capturar original con paleta, alfa, estado y geometría nativa.
2. Identificar plano, dirección del objeto, degradados, sombras y detalles.
3. Incluir explícitamente esos límites en el prompt de generación/edición.
4. Revisar alfa exterior y fondos claro/oscuro; revisar manualmente perspectiva
   de cada componente. Una validación de PNG no demuestra fidelidad artística.
5. Comparar silueta y disposición contra original antes de aprobar la pieza.
6. Probar dentro del HUD con animación, oclusión, fundidos y valores variables.
7. Registrar aprobado/pendiente/rechazado con motivo. Las piezas bonitas pero
   incompatibles con la perspectiva siguen pendientes, no completan una barra.

## Referencias de PS2/Re:CoM

Dirección vigente tras revisión del usuario: mantener el toon GBA y una
paleta similar a la del cartucho, con contornos y degradados suaves. El arte
PS2/Re:CoM sirve solamente de apoyo para entender qué representa cada pieza;
no trasladar su estilo, realismo ni perspectiva al HUD GBA. Esta decisión
supersede la propuesta anterior de usar sus cartas directamente como base.

El usuario autoriza buscar referencias HD de las mismas funciones y cartas.
Pueden aclarar detalles, pero no sustituyen la composición/perspectiva GBA.
Guardar procedencia y capturas originales localmente; no incorporar una
textura de otra versión sin revisar estilo, geometría y comportamiento.
Herramientas/formato documentados por el proyecto
[OpenKH](https://github.com/OpenKH/OpenKh), que incluye `OpenKh.Recom`, y su
[índice de herramientas](https://github.com/OpenKH/OpenKh/blob/master/docs/tool/index.md).
La disponibilidad de herramientas no demuestra que un recurso concreto haya
sido encontrado o extraído. Las referencias PS2 de estas cartas aún se investigan.

Referencia HD encontrada: [cartas de Re:CoM dentro de HD 1.5 + 2.5 ReMIX](https://www.spriters-resource.com/pc_computer/kingdomheartshd1525remix/asset/502588/),
subida por OCL, con listado de 397 elementos de `CARD/`. Es una referencia
de la colección HD para PC, no evidencia de identidad con las texturas PS2.
El formato [TM2 está documentado por OpenKH](https://openkh.dev/common/tm2.html).
No se han integrado archivos de ese paquete; falta comparar las piezas.
