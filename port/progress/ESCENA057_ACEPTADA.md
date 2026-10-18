# 057 Playa: mejora del pasto aprobada

El usuario aprobó también la mejora y pidió conservar explícitamente la regla
que ya sirvió en Monstruo: antes de prolongar un motivo cortado, encontrar una
copia completa en el mapa y emparejar las piezas existentes para recuperar su
mitad complementaria. Es una regla obligatoria de revisión de motivos, no un
retoque opcional al final. No trasladar coordenadas de una escena a otra.

Receta aprobada: `py -3 port/tools/scene057_polish_preview.py`.
BG3 izquierda destino x0..4,y18..22 fuente(x+8,y-4). Derecha x32..35,y5..11
fuente(x-15,y+4): nueve tiles con pasto coinciden con el motivo completo.
Se conservan las capas de pared; el extremo izquierdo ocupa también parte
del mapa original para cerrar el motivo. Header de integración actualizado.
La base anterior aceptada/mejorable sigue disponible en `c924ff625`.

El usuario considera la propuesta nueva un 95% correcta y la acepta como base
mejorable. Pendiente: cerrar correctamente las manchas de pasto reutilizando
sus piezas complementarias originales, siguiendo el método aprobado en 074.

Receta de base: `py -3 port/tools/scene057_tile_preview.py`.
Perfil de integración para main: `port/include/port/port_scene057_tiles.h`.
Base privada preservada: `build/scene-review/accepted057/`, con manifest SHA256.
Parte de la captura original `run1/0_057/before/shot_00100.bmp`; nunca carga
propuestas viejas de la playa. Cámara (7,43), vista 320x180, paleta activa de
`camera057/st.khst`.

BG3/BG2 izquierda fuente (+8,-4): la estructura sube hacia la derecha.
BG3 derecha (-8,+4), BG2 derecha transparente para no copiar el árbol único.
La palmera y personajes se conservan desde el encuadre original.

La mejora posterior ha sido aprobada y sustituye visualmente esta base.
Todavía no hay hook nuevo al PPU aquí; main integra los perfiles fuente.
