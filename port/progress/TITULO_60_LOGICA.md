# 60: barras del título hasta el borde

Causa: el título pide PORT_SIDE_COLOR. El presentador rellenaba sus márgenes
con un color plano, sin transportar las barras que llegan al borde GBA.

La corrección lee BG0 antes de componer (banda de PULSA START), y OBJ en el
pie de brillo. Solo prolonga filas con ocho píxeles iguales junto al borde;
no duplica letras, flechas ni la ilustración. Conserva la paleta viva, alpha
5/16 de BG0 y los efectos de brillo/fundido. Se habilita al terminar la intro
y se deshabilita al abandonar el título. El menú central no cambia.
El presentador usa PixelEdge para alinear las filas con la imagen original,
también con escalas fraccionarias. Ningún píxel GBA se modifica.

Comprobación: test_title_bands.py, EU/US/JP, 1920x1080, 2560x1080,
3840x1080, 1024x768, 1280x720 y 240x160. Compara antes/después de PULSA START
y menú/BRILLO: imagen nativa idéntica y continuidad del margen. La animación
de luz usa reloj de frames determinista en la prueba. No demuestra todavía
el título alternativo de Riku ni todos los idiomas EU por separado.

Evidencia: build/title60-regression/{region}-{size}/{before,after}/out_*.png.
Copia jugable separada: ReKHCoM-prueba-titulo-60 en la raíz; Probar titulo 60.bat
y Comparar titulo 60 anterior.bat (KHCOM_TITLE_BANDS_OFF=1 solo diagnóstico).
Aprobado visualmente por el usuario el 18/10/2026.

Comprobación adicional pedida tras aprobar: 12 comparaciones / 24 ejecuciones
EU/US/JP a 4K, vertical, 640x480 y 1920x997; 3 comparaciones / 6 ejecuciones
al redimensionar 640x480 → 3840x2160 en caliente. El caso de altura impar
reveló una columna negra final por truncar x y ancho fraccionarios en SDL
software: ambos extremos se redondean en la misma cuadrícula. Las barras y
el fondo ahora llegan al último píxel; el rectángulo original sigue idéntico.
