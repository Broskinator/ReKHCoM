# 057 Playa: aceptada, mejorable

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

Solo el usuario puede aprobar que la mejora posterior sustituya esta base.
Todavía no hay hook nuevo al PPU aquí; main integra los perfiles fuente.
