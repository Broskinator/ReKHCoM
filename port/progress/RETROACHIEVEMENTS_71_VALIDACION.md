# 71 — avances de Codex, 19/10/2026

Base recibida de Claude: 21/90 oficiales activos en Windows/EU (softcore),
69 pendientes de validar; 16 retos locales en EU/US/JP. Esto reemplaza las
notas históricas del 27 que indicaban evaluación oficial totalmente apagada.
No se vuelven a aplicar los roots de batalla: ya están en 3a27f7aa9.

## Bloqueo de modificadores antes de evaluar

El frame nuevo ejecutaba KhRaPump antes de invalidar la sesión por un truco
activo. Eso permitía una evaluación oficial en el primer frame modificado.
Se invalida primero; apagar luego el truco no recupera premios en esa sesión.

Comprobación: test_official_frame_guard.py enlaza el port_achievements.c real
con un espía RA sin transporte ni credenciales. Los 12 casos EU/US/JP ×
one-hit/HP infinito/desbloqueo Riku/modo PS2 prueban frame limpio, primer frame
modificado y siguiente frame tras desactivar el truco. Ningún premio real ni
petición de red se genera. build/official-frame-guard.log.

## Trabajo que sigue

Validación de las cadenas de batalla/cartas, memoria fija del heap, Pooh,
piso e IWRAM contra el original. No se amplía sOfficialVerified por pasar un
mock o compilar. La ampliación requiere evidencia de los campos reales.
Fondos 61/62 y 16–24 apartados por el usuario. Commits de avances comprobables
aproximadamente cada cinco minutos (última instrucción, sustituye tres minutos).

## Estado original sin abrir BizHawk

ra_bizhawk_state.py lee Core.bin.zst del savestate privado existente con la
biblioteca Zstandard ya instalada. Valida longitud, revisión, código y CRC32
contra la ROM exacta antes de extraer los 32 KiB IWRAM y 256 KiB EWRAM.
No toca el savestate ni SRAM del usuario. Salida privada limitada a build/.
Comprobado con el savestate USA del usuario (revisión mGBA 9).
Esto aporta RAM original reproducible; no demuestra por sí solo paridad con
el port ni valida las condiciones que requieren estar en combate.

Fuentes del formato: MGBAHawk.IStatable.cs de TASEmulators/BizHawk y
https://github.com/mgba-emu/mgba/blob/master/include/mgba/internal/gba/serialize.h
La extracción requiere --rom y --zstd explícitos; no copia datos privados al repo.

## Corregir la identidad del anclaje de combate

Los literales de BgFxIsBlocked son EU 0x02034A28, US/JP 0x02034928.
Su cuerpo opera sobre sBgFx (src/btl/btl_effect.c), no sobre gBtlWork.
La proyección anterior publicaba gBtlWork en esa dirección: una dirección
correcta con el objeto equivocado. PortRaBgFxWork devuelve ahora sBgFx, sin
modificar su vida ni lógica. Se conserva la restricción a punteros de heaps GBA.
Los tests atan explícitamente el testigo Thumb a su propietario C para evitar
regresar a la confusión entre ambas variables. Dirección y bytes parciales se
comprueban en EU/US/JP. No amplía por sí sola los 21 logros oficiales activos.

## Medidor de Pooh: campo escalar, no puntero

GetPooGaugeFrame lee gPoohGauge como u16. Los literales ROM dan EU
0x0203C9D4, US 0x0203C3E4 y JP 0x0203C354. Se proyectan ahora sus dos bytes,
con pruebas de lecturas parciales y protección de bytes vecinos por región.
El estado original USA conserva gauge=3 en esa dirección. La prueba del
cartucho valida el literal y su propietario C; la suite nativa sigue pasando.
El logro 160372 sigue fuera de sOfficialVerified hasta cerrar su grafo completo;
no se contabiliza como desbloqueo ni se envían premios en estas comprobaciones.

## Activado 160372: 22/90 oficiales (24,4 %)

Cerrado el grafo de Pooh con su definición pública real descargada: 300 frames
comparan el lector nativo (PortRaReadMemory y proyección de globals reales)
con una ventana GBA de referencia independiente. Una finalización válida
produce una activación; medidor vacío y dos recargas producen cero. El test
usa la biblioteca rcheevos fijada, no reimplementa las condiciones.
Comando: python port/tools/test_ra71_scalars.py --achievement 160372. Hash de la definición comprobada:
51613738a01ad0a544eb6327a69b61554c85d72db4e6249c20ac9bfa5bd40855.
La identidad/dirección de gPoohGauge se comprobó en EU/US/JP por código de ROM;
la activación oficial solo se añade para EU, como el resto del set admitido.

Se incorpora 160372 a sOfficialVerified. Total 22 activos de 90, quedan 68.
No se presenta el test como un premio real obtenido, una partida completa en
Bosque de los Cien Acres ni una prueba física: son condiciones reales con
snapshots controlados y memoria nativa. US/JP mantienen retos locales.

## Activado 15582: 23/90 oficiales (25,6 %)

0x0203CB86 es gMapFloorState.room (byte +6), no el número de piso. En las tres
ROM SetCurrentMapRoom carga la base de gMapFloorState y ejecuta STRB r0,[r4,#6].
Bases EU 0x0203CB80, US 0x0203C590, JP 0x0203C500. La proyección copia solo el
byte room y sus tests verifican que no toca los vecinos.

El mismo runner de escalares prueba 15582 con su definición oficial real:
500 comparaciones de frames. Una transición válida activa; sala incorrecta,
mundo anterior incorrecto, bandera excluyente y estado incorrecto no activan.
Comando: python port/tools/test_ra71_scalars.py --achievement 15582.
También conserva las 300 comparaciones de Pooh. No se publican los datos
privados ni se envían premios. Se añade 15582 a la lista EU, total 23/90.
Quedan 67 oficiales; cadenas de batalla/heap continúan pendientes de paridad real.
