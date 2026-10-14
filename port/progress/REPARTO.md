# Reparto de pendientes — 13 de octubre de 2026

Responsabilidades propuestas por el usuario para trabajar sin solaparse.
Esta lista no implica que se haya enviado una tarea a Claude.
Todos los puntos siguientes están pendientes salvo indicación explícita.

## Codex: bugs prioritarios y acabado visual solicitado

- [ ] Cristal del castillo: desaparece tras abrir/cerrar repetidamente su menú.
  Reproducir en vista panorámica y 16:9, corregir el ciclo de vida de sus tareas
  y recursos, y verificar varias interacciones. Estado 4 desaparecido;
  estado 8 actual frente al cristal, antes de desaparecer.
- [ ] Cian en selección de carta de puerta (estado 7) y animación posterior
  de apertura (estado 9, pulsar A). Corregir ambos y sus transiciones.
- [ ] Headers de Ver barajas, Añadir/Quitar, Naipes mapa, Estado/magias,
  Mapamundi, Naipes mundo y Diario: dibujar sus extensiones desde los
  elementos de fondo/arte originales. No tomar columnas de la imagen final,
  que contiene el guante, letras e iconos. Verificar cursor arriba y abajo.
- [ ] Tabs del ojo a izquierda y derecha: extender relleno y contorno propios,
  conservar el ojo y eliminar los rastros blancos del guante.
- [ ] Sustituir el estirado de píxeles donde sea viable por patrones o piezas
  de fondo. Conservar el patrón inferior de barajas y el de Mapamundi,
  que el usuario aprobó. Revisar el píxel residual del margen derecho.
- [ ] Entradas/salidas de submenús: headers sin ecos ni distorsión durante
  el movimiento. Mapamundi: footer completo desde el inicio y al volver.
- [ ] Naipes mundo: extensión del fondo mediante un fundido de pixel art
  escalonado/tramado basado en la paleta original, sin estirar la ilustración.
- [ ] Entrada a combate: fondo difuminado como en las películas para llenar
  la ventana, manteniendo la animación central en su proporción.
- [ ] Subida de nivel: conectar header/footer; eliminar sprite/fondo que
  aparece antes/después y al seleccionar un stat. Estado 3 antes de ganar EXP.
  La antigua ranura 8 de subida de nivel fue reemplazada por el cristal;
  reproducir desde 3 y guardar una copia aislada del punto de subida de nivel.
- [ ] Medir el stutter al permanecer en subida de nivel y corregirlo si se
  reproduce. Usar tiempos de fotograma y lógica, con pacing normal.
- [ ] Ampliar la prueba de crashes al alternar submenús/edición, y comprobar
  que los avisos de tutorial quedan delante del texto de las cartas.
- [ ] Revisar después qué entradas de PENDIENTES.md quedan resueltas por
  estas correcciones; actualizar evidencia y porcentajes y hacer push.

## Para Claude: resto del proyecto, fuera de este bloque

- [ ] HD text restante: inventario de números y textos aún en pixel art,
  reemplazo opcional de los que son texto y cobertura de idiomas EU/US/JP.
  Coordinar antes de modificar prioridades/oclusión del compositor.
- [ ] Validación de juego completo de Sora y Reverse/Rebirth en las tres
  regiones; las baterías de eventos/combates son pruebas limitadas,
  no sustituyen una partida completa. Anotar problemas y rutas reproducibles.
- [ ] Preservación: reproducir y completar los casos de BUGS_GBA.md en EU/US/JP;
  distinguir técnicas originales, anomalías del código y defectos del port.
  Coordinar cualquier cambio de gameplay antes de implementarlo.
- [ ] Audio: comparación con referencias de hardware/emulador de PSG,
  mezcla, FIFO, música y efectos; documentar diferencias medibles.
- [ ] Launcher e instalación: cerrar pruebas de importación de las tres ROM,
  opciones, controles, idiomas, instalación limpia y recuperación de errores.
- [ ] QoL pendientes previamente propuestos: devolver cartas acumuladas,
  aviso de puntos moguri, bancos de partidas, revancha y repetir tutoriales.
  Revisar alcance y dependencias antes de implementar extras.
- [ ] Publicación: preparar paquete/instalador, documentación y v0.5 cuando
  ambas líneas de trabajo estén verificadas. Coordinar la release final.

## Reglas compartidas

- No modificar simultáneamente los mismos archivos; Codex trabaja principalmente
  en map/map_tasks/map_cell, card_level_up, perfiles de menú, PPU y presentación.
- No sobreescribir ejecutables o estados que el usuario esté usando. Se preservó
  la build compatible con sus estados recientes en build/priority-reference.
- Las ranuras 3/4/7/8/9 recientes están en build/pcwork/states, no build/pc/states.
  Todas cargaron correctamente con la copia de referencia.
- Probar en directorios aislados. No modificar partidas/configuración del usuario.
- No subir ROM ni recursos extraídos al repositorio.
- Subir porcentajes sólo con evidencia y distinguir implementado de verificado.
