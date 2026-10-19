# Cristal del primer cuarto: sigue en mala posición (abierto, 19/10)

Para el Claude que lo retome. Prioridad crucial del usuario: «no quiero tener salas que no
sean posibles en GBA en mi port».

## Informe del usuario

- 19/10 ~10:50, con v1.9.9 (`Probar ReKHCoM v1.9.9.bat`) sigue viendo el **primer cristal**
  (la primera sala de cartas tras el tutorial, Moment's Reprieve en Ciudad de Paso) en mala
  posición. Según él, en el GBA aparece siempre en el mismo lugar.
- Su captura (v1.9.9) muestra la sala con la plataforma elevada, la escalera a la izquierda y
  un bloque de casa en el centro. El cristal aparece **al fondo de la plataforma**, arriba a la
  derecha del bloque, junto a la pared trasera. Eso es lo que considera mala posición.
- Dejó un **savestate en la puerta, antes de que se genere la sala**:
  - `ReKHCoM-v1.9.9/ReKHCoM-v1.9.9-windows-x86/states/slot1.khst` (port US, 10:50)
  - copia intocable: `backups/crystal-door-20261019/` (slot1.khst + kh_com.sav)
- La noche anterior dejó otro savestate en `ReKHCoM-v1.9.8/.../states/slot1.khst` con el
  mismo problema. Un harness sobre él dio `room=254 seed=00000000`, con el cristal en
  (155648, 73728). Una semilla 0 es sospechosa: vale la pena investigarla.

## Lo que está probado (y lo que no)

1. **Código.** `CreateMapRoom` pone `floorRoom->seed = gFrameCounter * gFrameCounter`.
   - `gFrameCounter` está en 0x03007480 (EU/US) y sube una vez por vuelta del bucle principal.
   - El cristal lo coloca `MapGmkPlaceGmk04` → `MapGmkFindSpot` con esa semilla.
   - Las salas fijas (`ROOM_FLAG_FIXED_ROOM`, vestíbulos y salas de historia) usan las tablas
     de ROM `MapFixCreateGimmicks`.
   - Hay 4 llamadas a `CreateMapRoom`: puerta con carta, puerta del tutorial, vuelta de sala
     de historia e inicio de piso.
2. **Paridad con la misma semilla.** `crystal_parity.py` toma la partida del usuario y cambia
   solo la semilla guardada (+0xE8 en los bloques 0x2F20 y 0x3E34, con `save_reseed.py`).
   - Resultado: el original (BizHawk US) y el port ponen el cristal en la misma posición en
     4/4 semillas: 0x2242C2F1, 0x3A4A4A40, 0x5F5E1000 y 0xA344A3F9 (3 posiciones distintas).
   - **Conclusión:** con la MISMA semilla, la colocación es idéntica.
3. **NO probado.** Que en el GBA la semilla de esa sala cambie realmente según el frame en
   que se entra. La prueba limpia (movie `.vbm` desde el encendido, entrar a tiempo y 37
   frames tarde) quedó sin resultado concluyente.
   - Las pruebas anteriores con BizHawk estaban contaminadas: BizHawk identifica el juego por
     hash y cargaba la SaveRAM del usuario.
   - Si el usuario tiene razón (en el GBA siempre igual), entonces hay algo que no entendemos.
     Opciones:
     - `gFrameCounter` en el GBA tiene un valor determinista al llegar a esa puerta, por
       ejemplo porque se reinicia o se detiene en algún punto, y en el port no;
     - la primera sala usa otra vía, por ejemplo la del tutorial;
     - el port avanza `gFrameCounter` en momentos donde el GBA no lo hace: menús del port,
       escenas saltadas, fundidos, carga de estado.

## Qué hacer

1. Cargar el savestate de la puerta en el port y registrar `gFrameCounter` y la semilla al
   crear la sala. Repetir esperando N frames antes de entrar: ¿cambia la posición?
2. En BizHawk (US), con la SaveRAM del usuario apartada (ver la regla de abajo), llegar a
   esa misma puerta y entrar en dos frames distintos.
   - Comparar con `bizhawk_seedwatch.lua` / `bizhawk_seedtrace.lua`.
   - Si el original da siempre la misma semilla, buscar quién fija o reinicia `gFrameCounter`
     (watchpoint de escritura en 0x03007480) y replicarlo en el port.
3. Revisar también que no sea solo de **dibujo**: la paridad compara la posición del objeto,
   no dónde se ve en pantalla en la vista ancha.
4. Revisar el caso `seed=00000000` del savestate de la v1.9.8.

## Reglas

- **SaveRAM de BizHawk del usuario**
  (`toolchain/bizhawk/GBA/SaveRAM/Kingdom Hearts - Chain of Memories (USA).SaveRAM`):
  respaldo en `backups/bizhawk-saveram-20261019`. Apartarla antes de cada prueba y
  restaurarla con verificación sha1, como hace `crystal_parity.py`. Hoy el sha1 es
  fd1edccb9cbc.
- La build US lee `kh_com_us.sav`; la EU lee `kh_com.sav`.
- No usar lógica propia del port: corregir dentro del flujo del juego
  (ver `LOGICA_APROBADOS.md`).

Herramientas: `port/tools/crystal_parity.py`, `save_reseed.py`, `bizhawk_reseed.lua`,
`bizhawk_seedwatch.lua`, `bizhawk_seedtrace.lua`; harness `KHCOM_MODE=N:gmkcount`
(registra el cristal y la semilla) y `N:crystalview`.
