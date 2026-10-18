# Editor de saves y cheats — lógica de implementación

Implementación de Claude integrada en `b7d491ef5` (editor) y `2fdca4ca7`
(cheats). Este documento conserva las reglas y el checklist de revisión pedidos
por el usuario; su existencia no demuestra que cada prueba de cierre ya pasó.
Requisitos: SAVE_EDITOR_Y_CHEATS.md. Integrador: Codex; transferencias según
regla de 10 min.

## 1. Fuentes contrastadas y límites

Referencia externa consultada: RyudoSynbios/game-tools-collection, commit
`a63d37d1db44acdb4a5d58ff41d66362c5cbe35c`, módulo
`src/lib/templates/kingdom-hearts-chain-of-memories/saveEditor/`.
Su template separa Sora/Riku y usa checksum por bloque; su orden de pestañas
no es el orden del índice nativo. Usarlo como referencia, no trasladar todos
sus offsets/valores máximos sin contrastar. Revisar y conservar licencia MIT
si se reutiliza código. El módulo implementa UI Svelte: el launcher es ImGui/C++.

Fuentes nativas que mandan sobre el template:

| Regla | Fuente |
|---|---|
| SRAM, dos copias de cada bloque, checksum | src/save.c, include/save.h |
| Layout, cuatro archivos lógicos | include/save_types.h, src/save_data.c |
| Importación, checksum y escritura temporal existentes | port/decompui/save_import.h, save_file.h |
| Capacidad total combate 999, baraja 99, tres barajas | include/save_types.h, include/card/card_types.h |
| Máximo 99 copias de un ID (incluye premium del mismo ID) | CountCardsById / AddCardToCollection, src/card/card_map_anim.c |
| Mapa: 270 contadores, 9 por entrada, 99 normales totales | AddMapCard / CountRegularMapCards, src/card/card_worldselect.c |
| HP 560, CP 9999, AP/DP 999, nivel 99 | src/player_progression.c |
| Resolución de daño y muerte normal | ApplyBtlObjHit, src/btl/battle_runtime.c |

**Decisión del usuario:** conservar 999 en total y formato original. No migrar a
colección extendida, no borrar cartas para hacer espacio. Los máximos de campos
son límites del código, no una promesa de alcanzar todos jugando naturalmente.

## 2. Contratos y separación de capas

Crear un modelo C++ sin ImGui ni acceso global al juego, por ejemplo
`port/decompui/save_editor_model.h`. UI y escritura son capas separadas:

```text
Inspect(image, region) -> SaveIndex | Error
OpenSlot(image, logicalSlot) -> Snapshot | Error
PlanEdit(snapshot, commands) -> ChangeSet | Error
Validate(candidate, region) -> Validation
Commit(path, expectedOriginalHash, changeSet) -> Result
```

Snapshot contiene SRAM original completa, hash, región seleccionada, banco,
archivo lógico y sus copias válidas. ChangeSet contiene bytes cambiados,
campos antes/después, bloques afectados y errores de capacidad/referencias.
PlanEdit no escribe ni cambia configuración. Errores se muestran en la pestaña.

Usar lectores/escritores little-endian con comprobación de rango, no reinterpretar
SRAM como structs de C++ del launcher: alineación y ABI pueden diferir. Obtener
offsets/tamaños de las estructuras compiladas con las opciones del port y
contrastar con los offsets conocidos; añadir asserts a la prueba del modelo.

## 3. Reconocimiento y archivos lógicos

La SRAM manejada por el port son 65536 bytes. Las firmas tienen 24 bytes:
EU usa KHCOM_BACKUP_VER00000013, US/JP KHCOM_BACKUP_VER00000012. US y JP no
se distinguen solo por firma: usar región del launcher/cartucho y sus recursos;
no anunciar autodetección entre ambas. Importadores existentes pueden convertir
contenedores; editar el SRAM normalizado después de importación explícita.

| Bloque | Copia A | Copia B | Tamaño |
|---|---:|---:|---:|
| Cabecera | 0x0010 | 0x0050 | 0x40 |
| Quicksave/sistema | 0x0090 | 0x0FA4 | 0xF14 |
| Riku 1 | 0x1EC0 | 0x22D8 | 0x418 |
| Riku 2 | 0x26F0 | 0x2B08 | 0x418 |
| Sora 1 | 0x2F20 | 0x3E34 | 0xF14 |
| Sora 2 | 0x4D48 | 0x5C5C | 0xF14 |

Índice lógico de cabecera: Sora1=0, Sora2=1, Riku1=2, Riku2=3.
No usar ese índice como posición física de la tabla de arriba. Banco 1–5 es
el archivo elegido por SavePath, distinto de archivo lógico y copia redundante.

Comprobar firma y Checksum(block)==0. Reproducir la prioridad A→B de SaveLoad.
Si solo una copia es válida, mostrar recuperación; nunca elegir por supuesta
fecha que el formato no registra. No crear partida dentro de una ranura vacía
rellenando flags: eso requiere un flujo aparte de nueva partida.

## 4. Activación y transacción

Estado: Bloqueado → Confirmación → Archivo cargado → Borrador → Revisión → Guardado.
Cancelar confirmación vuelve a Bloqueado sin cambiar archivo/configuración.
Aviso: «Editar una partida cambia sus datos de forma permanente. Se creará una
copia antes de guardar. ¿Activar el editor?» Botones Cancelar / Activar editor.
Aceptar el aviso no escribe nada. Confirmar cambios es una acción posterior.

1. Resolver región/banco/ruta mediante el mismo SavePath que el launcher.
2. Cargar snapshot y validar. Evitar escribir mientras el juego usa ese archivo.
3. Aplicar comandos en copia, mostrar diff semántico y capacidad restante.
4. Antes de escribir, releer y comparar hash con snapshot. Si cambió, detener y
   recargar; no sobrescribir el guardado recién hecho por otro proceso.
5. Actualizar solo el bloque del personaje elegido y las dos copias de ese bloque.
   Actualizar en ambas cabeceras solo su resumen (nivel/HP no van en los mismos
   campos; resumen contiene piso, mundo, nivel y tiempo). No cambiar los otros
   resúmenes, flags de historia/título, quicksave o metadatos Re:CoM del port.
6. Checksum en offset +0x18: ponerlo a cero y calcular con RekhcomSave::Checksum.
   Es suma complementada de palabras LE con acarreo, no CRC ni suma truncada.
   Verificar otra vez checksum cero en bloque y cabeceras afectados.
7. Validar candidato, crear backup exitoso y usar Replace (temporal en misma
   carpeta + sustitución). Ante error conservar original. Releer destino y
   comprobar hash/cabeceras; registrar backup, nunca tokens ni datos personales.

La apertura de la pestaña y selección de ranura no cambian GAME_FLAG_SECOND_FILE:
el editor no convierte otra ranura en la última jugada por mirar sus datos.

## 5. Catálogo, inventario y barajas

ID de carta = bits 0..11; 0xFFF es hueco vacío de colección, 0xFFFF centinela
de baraja. Bits 12..14 indican uso en barajas y bit15 premium. No confundir
un ID con índice en colección. Los Deck.cards apuntan a índices de colección.

Catálogo regional desde gCardDefs: nombre localizado, categoría, valor, CP,
premium permitido y recurso visual. Excluir amigo/gimmick/no coleccionables.
Cartas enemigo/jefe solo en variantes definidas; no inventar valores 0..9 para
cualquier categoría. Riku no recibe inventario/barajas de Sora.

«Añadir hasta N»: contar copias por ID, limitar a 99 y a huecos libres dentro de
999; mostrar cantidad solicitada y cantidad que cabe. Si un lote no cabe,
proponer cantidad ajustada en revisión y exigir elección explícita, sin borrar
ni truncar silenciosamente. No confundir gCardCount (rango del array utilizable)
con número de entradas ocupadas; recalcular conforme a la lógica nativa.

Conservar índices al añadir; al borrar una carta usada pedir quitarla de las
barajas afectadas y recalcular referencias/coste/flags. Máximo 99 por baraja,
tres barajas; validar cpCost contra definición regional y CP disponible.
No compactar colección sin remapear todos los índices. La primera entrega puede
limitarse a añadir/editar copias para reducir riesgo en eliminación/reordenado.

Mapa: elegir tipos/valores válidos, hasta 9 por entrada y hasta 99 normales;
llaves/eventos se manejan aparte con su lógica nativa. Un botón «reponer» llena
solo la selección que cabe. No escribir 99 en los 270 bytes: desborda reglas y
contadores intermedios de ocho bits. No desbloquear historia al reponer cartas.

## 6. Sprites ×1 y presentación

Decodificar desde el cartucho verificado (CardDef gfx/tiles/palette y definición
de mapa), con el mismo algoritmo probado del port. Cache por hash de ROM,
región, ID y variante. Mostrar tamaño nativo en píxeles lógicos, filtro nearest;
respetar DPI del launcher sin mezclar densidades ni inventar arte de cartas.
Si falta ROM/recurso, mostrar placeholder y nombre, no cargar una imagen ajena.
Lista filtrable por categoría/nombre/valor y resumen de copias/espacio. Vista
previa y datos de cambio a la derecha; Guardar cambios solo tras validar.

## 7. Cheats temporales frente a cambios guardados

Cheats de sesión: vida infinita y un golpe. Opciones persistentes en INI con
defaults OFF, controles en launcher y juego, texto que indique cuándo actúan.
Cambios permanentes (nivel/bonos/cartas) son comandos explícitos, con la misma
vista previa/validación del editor; no reescribir inventario cada fotograma.

Vida infinita: en la resolución de daño al actor controlado, anular únicamente
daño positivo y estados de muerte derivados, respetando curación y eventos.
Auditar también daño de campo y escrituras directas de HP. No resucitar durante
cambio de modo ni tocar enemigos/compañeros. Mostrar que el save conserva HP
válido; apagar no deja HP cero ni flags de derrota heredados.

Un golpe: en ApplyBtlObjHit, después del cálculo de dificultad y antes de restar
HP, para un impacto dañino válido sobre enemigo hostil, llevar daño a su HP
actual sin overflow. Mantener la ruta normal de derrota/animación/premios.
No destruir tareas ni poner HP=0 desde un bucle. Respetar invulnerabilidad,
escudos y transiciones de jefes; no saltar fases de historia ni dañar objetos
no hostiles. Probar Sora/Riku y jefes con múltiples partes.

Nivel máximo completo: nivel 99, umbral EXP coherente con
AdvanceLevelExpThreshold, HP actual/máximo dentro de 560, CP dentro de 9999.
Sora aprende todos los sleights definidos sin alterar progreso de historia;
usar la semántica nativa de LearnStock y límites de sus bitfields. Riku conserva
su sistema de baraja y bonos AP/DP (límites 999), sin CP/sleights de Sora.
No fabricar progreso de mundos, tutoriales o Game Clear. Recalcular resumen.
El valor de CP que resulta de otorgar todas las subidas es 2725 (275 de inicio
+ 98×25), no el tope técnico 9999; no dar CP adicional ni llamarlo build legal
obtenible con una selección natural de 98 subidas, que reparte HP/CP/sleights.

Al integrar RetroAchievements (27), comunicar modificaciones al cliente y
deshabilitar envío de logros mientras cheats/editor afecten esa sesión; activar
un flag no basta para restaurar integridad al apagarlo sin recargar limpio.

## 8. Pruebas y cierre real

Modelo: EU/US/JP; cuatro archivos y cinco bancos; firma/checksum roto; A mala/B
buena; copias válidas diferentes; vacío; tamaño incorrecto; versión ambigua.
Idempotencia no-op: bytes idénticos. Guardar cambia únicamente whitelist de
bloques/headers; resto y ranuras ajenas idénticos. Checksum aceptado por el juego.

Inventario: 0/998/999, copia98→99/99→rechazo, premium, ID inválido, amigo/gimmick,
referencias usadas en una/tres barajas, total99 de mapa y entrada9. Comprobar
coste y nombres/deck regional, ninguna escritura fuera de rango.

Transacción: cancelar no escribe; backup fallido no escribe; modificación externa
detectada; fallo de temporal/reemplazo conserva original; reapertura idéntica.
No usar partidas originales del usuario como fixture de escritura.

Runtime: cheats OFF idénticos al baseline; ON solo actor/enemigos indicados;
curación, muerte/derrota normal, bosses multifase, guardado/carga, Sora/Riku y
EU/US/JP. Pestaña: warning, sprites ×1, teclado/mando y traducciones.
Cerrar 30/47 tras revisar la implementación integrada, compilar y completar la
matriz de preservación, límites, regiones y reapertura con evidencia. No atribuir
al editor el paso de una prueba que solo compiló o abrió la pestaña.

## 9. Evidencia verificada en main

El 18/10, `python port/tools/test_save_editor.py` pasó con el binario EU compilado
desde el main integrado. Crea Sora archivo 1 desde un punto de guardado real,
usa el probe del modelo para nivel 50 / HP 400 / CP 1500 / moguri 12345 / cartas,
comprueba checksum de las dos copias y cabeceras, asegura que los bytes ajenos no
cambien y vuelve a cargar los valores en el juego. Salida: `cards=19/999 maps=6`
y `PASS: edited file 1 loads in the game with its new values; nothing else changed`.

Alcance comprobado: modelo/probe y reapertura de un save de prueba EU Sora archivo
1. Esta prueba no automatiza clicks del launcher ni demuestra Riku, archivos 2,
los otros bancos/regiones, sprites en la pestaña, fallo/rollback de filesystem o
persistencia completa del editor. No inferir esas pruebas desde el pase anterior.
