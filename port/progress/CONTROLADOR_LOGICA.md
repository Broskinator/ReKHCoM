# Controles: mando y símbolos Xbox — lógica del punto 48

Implementación integrada desde Claude en `b3a0f1e7d` y punto 48 cerrado en
`5e23665c7`, después del traspaso por inactividad. Este checklist registra la
lógica solicitada por el usuario y guía la revisión; no sustituye la validación
física de mandos. Complementa el remapeo del punto 25. No cambiar asignaciones.

## Estado y representación

Una sola tabla ButtonVisual relaciona el binding SDL con nombre traducido,
símbolo y región del dibujo: A/B/X/Y, LB/RB, LT/RT, Back/Start, cruceta, LS/RS.
Los ejes se decodifican con PORT_PAD_AXIS_BASE: índice /2, signo en bit 0.
LT/RT positivos iluminan sus gatillos; los ejes de stick iluminan su dirección.
Bindings de teclado no se convierten en botones Xbox ni iluminan un botón ajeno.

Separar dos estados visibles: acción enfocada (resaltar su binding actual) y
captura activa (resaltar también lo que se está pulsando hasta confirmar).
Al terminar/cancelar captura, volver al foco de la fila. Mantener Esc/Delete,
restauración y advertencias de asignaciones repetidas ya implementadas.

## Arte y UI

- Silueta de mando inspirada en la distribución Xbox 360, dibujada con píxeles
  de una rejilla lógica fija. No copiar logotipo ni fotografía.
- Render propio con ImGui: rectángulos de la rejilla a escala entera, sin AA en
  la capa del dibujo. Posiciones de sticks asimétricas, cruceta, cuatro botones,
  hombros/gatillos y centro reconocibles.
- Un solo origen y factor de escala entero para el mando y sus regiones; si no
  cabe, reorganizar mando/lista, no usar píxeles de distintos tamaños.
- Botones A/B/X/Y con letra y forma además de color; hombros/gatillos rotulados.
  Mostrar junto a cada binding su símbolo Xbox y nombre, no solo un enum SDL.
- Resaltado por contorno/luz de paleta, manteniendo letra visible. Sin animación
  continua que dificulte leer. Texto de acción y estado de captura próximos.
- Primera integración en PageControls de port/decompui/decompui.cpp; reutilizar
  tabla de nombres en el menú del juego cuando sea posible. No añadir imágenes
  generadas a la ROM ni alterar recursos originales.

## Criterio de cierre

Comprobar cada binding (incluidos LT/RT, sticks, cruceta y sin asignar), cambio
de fila, captura, cancelación y reconexión. Asignación antes/después idéntica
salvo edición deliberada; persistencia tras reinicio. Revisar capturas con UI
español/inglés, ventanas pequeña/grande y escala de Windows. Mando virtual
verifica mapeo; mando físico queda indicado si no está disponible. Compilar
launcher y EU/US/JP, registrar evidencia y push.

## Verificación física, 18/10/2026

En este equipo SDL3 3.4.18 inicializó el subsistema de gamepad correctamente,
pero `SDL_GetGamepads` devolvió cero dispositivos. Windows lista USB/HID, pero
ninguno aparece ante SDL como gamepad y, por tanto, no se atribuye a ellos una
prueba de mandos. El cierre físico sigue pendiente hasta conectar uno reconocido
por SDL y registrar su nombre/VID/PID y los botones/ejes comprobados. Se conserva
la prueba virtual existente para regresión de mapeos y gatillos.

El launcher añade un panel de comprobación con identidad SDL/VID/PID, lecturas
LT/RT, pulsación/liberación de botones, sticks/centrado y gatillos simultáneos.
Durante la prueba desactiva navegación del mando para que los botones no cambien
opciones por accidente; ratón/teclado siguen disponibles. Exporta únicamente
observaciones a controller_report.txt, identifica mandos virtuales y no declara
certificación física. Al desconectar detiene la prueba. La apertura del mando
ya no depende de tener una celda de remapeo en captura: se enumera siempre y se
cierra su handle al desconectar/salir. La prueba física sigue pendiente de un
dispositivo conectado; el usuario fue consultado mientras continúa el trabajo.

### Dispositivo conectado por el usuario

El 18/10/2026 el usuario conectó su 8BitDo Ultimate 2C por dongle. Una nueva
enumeración SDL devolvió `gamepads=1`, `XInput Controller #1`, VID `2dc8`,
PID `310a`. Esto sustituye el resultado anterior de cero mandos para el estado
actual del equipo. Se abrió el launcher de revisión y se pidió al usuario
ejecutar el diagnóstico y exportar `controller_report.txt`. Hasta recibir ese
informe no se dan por comprobados botones, ejes, gatillos ni reconexión física.

### Informe físico recibido y nueva ubicación solicitada

El usuario guardó el informe y confirmó haber pulsado todos los botones y movido
los ejes. `build/achievements-review/controller_report.txt` registra SDL virtual=0,
VID 11720 / PID 12554 (2DC8:310A), down/released=32767/32767 (15/15),
sticks=255 (8/8 direcciones), centered=15 (4/4), LT=1/1, RT=1/1 y simultáneos=1.
Esta es evidencia de entradas reales; no demuestra reconexión, persistencia de
bindings ni todas las rutas dentro del juego.

El usuario aclaró que el dibujo debía estar en el menú Controles del juego.
La primera versión (3542f817d), reemplazada por el arte Gemini de abajo, fue:
`port_control_diagram.inc`, incluido por port_ui.c, dibuja una silueta propia
inspirada en Ultimate 2C sobre rejilla 96x64, contorno violeta oscuro, tres planos
de carcasa, botones rotulados y pozos de sticks. Va bajo la lista, encima de ayuda,
con una sola escala entera. El menú del mando usa separación de filas de 14 px
para conservar toda la lista y reservar 68 px; ventanas menores desplazan la lista.
El foco ilumina su binding y el signo correcto de un eje; captura añade entradas
del handle SDL ya abierto. No abre handles extra ni modifica bindings al dibujar.
Lista: A/B/X/Y, LB/RB, LT/RT, LS/RS y cruceta con flechas. Teclado conserva nombres
de teclas. Builds EU/US/JP y capturas reales del renderer en 1920x1080, 1016x570 y
480x320; evidencia privada build/controller-native-review. Aprobación artística
pendiente; no declararla aprobada automáticamente.

El salto de escala y la luz original del debug quedaron corregidos en 75a7efa60.
Detalles y evidencia: MENU_TRANSICIONES_LOGICA.md.

### Arte Gemini elegido y conectado al remapeo

El usuario aportó después Gemini_Generated_Image_lukhx5lukhx5lukh.jpg y pidió
usar ese dibujo, con transparencia, y darle las funciones del control. Se retiró
el fondo con imagegen integrado preservando el diseño elegido. Fuente transparente,
procedencia y prompt en port/assets/controller; el importador normaliza a 104x72,
48 colores y cobertura binaria sin dithering, y genera port_controller_art.inc.
El juego usa 7488 bytes de índices y 49 entradas de paleta, sin JPEG/PNG decoder
ni archivos externos que falten en paquetes Web/Android/consolas.

La máscara interactiva se alinea sobre cada botón de ese arte. Se conservan sus
letras oscuras al teñir; un borde luminoso de un píxel indica la selección. Las
cuatro direcciones de cruceta son regiones independientes; LS/RS distinguen
dirección de eje y pulsación de stick. Minus/plus/corazón corresponden a
Back/Start/Guide. Los pequeños botones decorativos restantes no inventan bindings.
LT/RT aparecen como pestañas porque están detrás en esta vista frontal; LB/RB
encienden pestañas y los topes visibles. Sus IDs internos quedan después de
SDL_GAMEPAD_BUTTON_COUNT, separados de los números reales de botones SDL.

El dibujo completo mide 104x79 con las pestañas. Reservar 80 píxeles mantiene
las 15 filas actuales del mando a altura lógica 320; se dibuja con escala entera
sin placa de fondo y se recorta la iluminación a píxeles opacos. La captura consulta
el handle existente y no modifica asignaciones al dibujar. Al reasignar, el foco
usa inmediatamente el binding guardado, con el nombre Xbox correspondiente.

EU/US/JP compilan. Capturas del renderer 1080p y 1016x570 en
build/controller-gemini-review: A, LB, LT y eje LS←. Se dejó copia jugable separada
ReKHCoM-prueba-menu-mando-20261018, con datos propios, y Probar menu y mando.bat
en la raíz. No pisa las carpetas de versiones ni sus partidas. La aprobación
artística queda para el usuario; las pruebas virtuales de presentación no sustituyen
el informe físico de entradas ya recibido.
