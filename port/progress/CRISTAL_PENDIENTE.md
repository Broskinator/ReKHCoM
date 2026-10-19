# Cristal de la primera sala: CERRADO (19/10, la lógica es la del cartucho)

El usuario veía el cristal (y los faroles) de la primera sala tras el tutorial en otro lugar que
en su emulador y creía que esa sala era fija. No lo es, tampoco en el GBA.

## Pruebas

1. **Código máquina del cartucho (USA).**
   - `MapGmkTutorialWaitCard` (0x080F1A10) carga `gMapFloorDefs[0].entryRoom` (0x0984C868),
     que en el piso 0 es la sala 1, y llama a `CreateMapRoom` (0x080DF640).
   - `CreateMapRoom` guarda como semilla `gFrameCounter²` (0x080DF66E).
   - Las 5 llamadas a `CreateMapRoom` del ROM coinciden 1:1 con la decomp.
   - Solo dos instrucciones del ROM escriben `gFrameCounter` (0x03007480): la puesta a 0 de
     `AgbMain` y el `++` de cada vuelta del bucle.
2. **Cartucho en marcha (BizHawk, SaveRAM del usuario apartada y restaurada, sha1 verificado).**
   - Partida nueva desde el encendido hasta la misma puerta del tutorial.
   - Misma carta y mismos botones: entrar de inmediato da la semilla 0x46112944 = 34286²;
     37 fotogramas después da 0x4636D944 = 34322².
   - Resultado: dos salas distintas (bloque, farol y cristal en otro sitio).
   - Evidencia: `build/bh-frames/gba_two_frames.png` y la imagen de la copia de prueba.
3. **El BizHawk del usuario.**
   - Su sala tiene la semilla 0x90FAECF1 = 49319²: la creó en el fotograma 49319 de esa sesión.
   - Siempre la veía igual porque cargaba un estado guardado dentro de la sala ya creada.
4. **Paridad.** Con la misma semilla, el port coloca el cristal exactamente donde el cartucho:
   6/6 semillas (`crystal_parity.py`).
   - La herramienta ahora también compara la lista completa de objetos de la sala (faroles,
     cajas…) con el harness `gmkall`.
5. **"Lógica Mapa GBA" (69) no interviene:** solo cambia la vista de enemigos (`MAP_ENM_VIEW`).

## Para el usuario

- Copia de prueba: `Probar cristal puerta 72.bat` (v1.9.9 tal cual, abre en su savestate de la
  puerta).
- Imagen: `ReKHCoM-prueba-cristal-72/cristal_gba_vs_port.png`.

Herramientas: `port/tools/crystal_parity.py`, `save_reseed.py`, `bizhawk_*.lua`,
`build/bh-frames/run.py` (corridas del cartucho con checkpoints propios, sin tocar los slots del
usuario).
