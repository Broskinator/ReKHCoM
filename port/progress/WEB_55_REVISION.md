# Web 55 / Logros 27 — revisión privada, 18/10

La web usa ahora la tipografía KHMenu y el lenguaje blanco/negro/rojo del launcher:
pestañas de juego, gráficos, sonido, mejoras, controles, editor y logros.
Opciones persistentes en INI/IDBFS; cambios admitidos se aplican en vivo y los
que necesitan reinicio se bloquean durante el juego. Teclado y mando reasignables.
Editor optativo con confirmación, copia original descargable, resumen, checksum y
copias redundantes, 99 por carta/999 total y protección de cartas en barajas.
Sprites originales a x1 leídos de la ROM privada. No incluye ROM ni sus imágenes.

Logros: catálogo de los 16 retos locales directo, búsqueda/filtros, progreso de la
región cargada y sprites gris/color. Comparte las definiciones nativas generadas
por gen_web_achievements.py. El progreso se persiste con IDBFS al desbloquear.
La conexión oficial de cuenta sigue limitada al launcher Windows y no se anuncian
premios oficiales desde web ni consola.

Pruebas: editor C++/JS 12/12 por byte; contratos runtime web; comprobación en
navegador de carga de cartucho EU, opciones en vivo, confirmación/editor, descarga
sintética nivel 50/HP400 con checksum correcto, búsqueda de logros (2 resultados
moguri), filtro conseguido vacío sin inventar progreso. Vista 1920x1080 y móvil
390x844. Capturas privadas build/web55-preview. Compilaciones Wasm EU/US/JP.

Entrega: servidor local 127.0.0.1:8765/web55-preview/; anterior web55-before/.
Pendiente aprobación visual antes de publicar. Los binarios que reconstruyen el
menú in-game se actualizan después de integrar la nueva entrada Logros.
