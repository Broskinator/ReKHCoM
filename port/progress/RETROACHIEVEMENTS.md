# RetroAchievements: viabilidad para Re:Chain of Memories PC

Fecha: 18-10-2026

## Conclusión

Sí es técnicamente viable ejecutar el set oficial de Kingdom Hearts: Chain of Memories (GBA) desde este port, al menos en modo local durante una primera integración. RetroAchievements publica el juego GBA ID 768 con un set base de 90 logros y un set adicional «Subset - Level 1» de 39 desafíos de jefes sin subir de nivel. La página oficial de hashes indica que el set base admite la ROM europea (MD5 `d369f791d86a969778d2c3a5278d544c`); el subset reconoce un archivo parcheado concreto (MD5 `02c38fb19dab302379ece65724fb4e0f`). Así que identificar las ROM EU/US/JP del port para el set base no se puede dar por hecho: solo comprobé que la lista registra la EU, y el Level 1 requiere la variante parchada.

El runtime `rcheevos` permite que el cliente entregue una función de lectura de memoria y obtenga el catálogo/sesión; no incluye transporte HTTP ni interfaz. El port reserva el espacio de direcciones GBA en el proceso y mantiene la RAM en `0x02000000` (EWRAM, 256 KiB) y `0x03000000` (IWRAM, 32 KiB), de modo que una función de lectura puede traducir las direcciones de logros GBA al mapeo del proceso.

La RAM accesible no basta por sí sola para reconocer un juego. El cliente debe calcular la huella de una ROM original soportada, iniciar una sesión y ofrecer el mismo mapa de memoria que espera rcheevos. Se debe validar que `rhash` identifique las copias EU/US/JP que use el port; una traducción de direcciones del huésped al mapa de memoria de rcheevos puede ser necesaria. Para no afectar la identificación, los logros se deben desactivar si se activan modificaciones que alteren datos/jugabilidad. El Undub que se está implementando cambia el contenido de la ROM en memoria, por lo que debe probarse expresamente y, por seguridad, excluirse inicialmente de Hardcore.

El set de la web está diseñado para la versión GBA identificada por sus hashes, no para prometer compatibilidad oficial de esta aplicación. La ruta más sensata es primero un modo local/offline para verificar que se leen las banderas y el progreso, después la identificación y compatibilidad con servidor, y por último consultar el proceso oficial de RetroAchievements antes de anunciar logros online o Hardcore. No se contactó a nadie ni se publicó nada.

## Referencia: RaceWave46

El proyecto que señaló el usuario es efectivamente [DomazinUS/RaceWave46](https://github.com/DomazinUS/RaceWave46), una recompilación de Wave Race 64 pública bajo GPL-3.0. Su implementación de RetroAchievements sirve como referencia de producto: integra el cliente `rcheevos`, muestra logros y avisos animados de desbloqueo, y tiene una capa de plataforma para comunicación con el servicio. La interfaz consultada usa sesión de cuenta y el flujo documentado funciona en Softcore; no debe describirse como logros completamente offline ni como soporte Hardcore.

Para KHCoMPC conviene conservar la idea general —tarjeta clara propia del juego, icono, nombre, descripción, puntos y una entrada/salida breve con sonido opcional— y no copiar recursos ni asumir que el código de Wave Race se puede trasladar tal cual. Antes de integrar una dependencia GPL-3.0 en este proyecto hay que revisar compatibilidad con la licencia y el empaquetado actuales. También hay que validar el set base y el set `Level 1` por separado: el segundo depende de una ROM parcheada con hash propio; no se puede activar por el mero hecho de que el juego original cargue.

## Capas necesarias

- `rcheevos`: evaluación de condiciones y lógica de runtime.
- `rhash` / `rc_client`: identificación de la ROM y sesión del juego.
- Adaptador de lectura: ventanas de memoria GBA del juego hacia las direcciones de consola que esperan las condiciones.
- Cliente HTTP(S), callbacks asíncronos, autenticación y almacenamiento seguro del token; no los aporta la biblioteca.
- Interfaz para login, lista/avisos de logros, pausa y errores.
- Política de integridad para ROM, región, savestates, cheats y las funciones experimentales; probar cada caso antes de habilitar Hardcore.

## Primer bloque integrado: lector acotado de RAM

`port/src/port_ra_memory.c` expone `PortRaReadMemory`, que lee únicamente
EWRAM (`0x02000000–0x0203ffff`) e IWRAM (`0x03000000–0x03007fff`). Rechaza
lecturas que cruzan el límite de una región o apuntan a MMIO/ROM/SRAM y pone a
cero el buffer en caso de rechazo. Esto implementa la frontera de memoria que
necesita el cliente; todavía **no** incorpora rcheevos, identificación/hash,
sesión HTTP, interfaz ni logros verificables.

La prueba `port/tools/test_ra_memory.c` reserva esas ventanas en sus direcciones
GBA reales y ejercita bytes de borde, lectura inicial, exceso de rango, MMIO y
lectura vacía. Compilada con el toolchain i686 de SDL/MinGW, pasa con
`PASS: RetroAchievements GBA RAM adapter bounds`. Siguiente paso: validar que el
hash del juego y las definiciones oficiales carguen, y después conectar esta
función al callback de lectura de `rc_client`.

También se comprobó por MD5 la ROM europea de 32 MiB que acompaña al Nightly
2 local: coincide con `d369f791d86a969778d2c3a5278d544c`, el hash EU anotado
para el juego 768. Las copias US/JP locales no coinciden con ese hash EU, como
se espera; sus hashes admitidos aún deben verificarse con la lista oficial.
La identificación futura debe calcularse desde la ROM original antes de que
las relocaciones nativas modifiquen palabras en la región mapeada. No hay un
parche Level 1 local disponible para verificar el MD5 `02c38fb19dab302379ece65724fb4e0f`.

## Fuentes primarias

- README y API de rcheevos: https://github.com/RetroAchievements/rcheevos
- Guía oficial `rc_client` (lectura de memoria, callbacks HTTP, autenticación y carga de sesión): https://github.com/RetroAchievements/rcheevos/wiki/rc_client-integration
- Requisitos oficiales para nueva plataforma/set (emulación, memoria expuesta, identificación única y servidor): https://docs.retroachievements.org/orphaned/system-x-wen.html
- Página oficial del set base de CoM para GBA, juego 768, 90 logros, y hashes compatibles: https://retroachievements.org/game/768 y https://retroachievements.org/game/768/hashes
- Set adicional Subset - Level 1: 39 desafíos, más hash admitido: https://retroachievements.org/game/768?set=40403 y https://retroachievements.org/game/768/hashes?set=40403
- Referencia abierta RaceWave46 (README y licencia GPL-3.0): https://github.com/DomazinUS/RaceWave46
