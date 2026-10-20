# Kit de depuración (20/10/2026)

Para encontrar fallos como el de Cloud en Android antes de que lleguen como reporte.

## Por qué

Las builds de escritorio (Windows y Linux x86) **recuperan** los accesos raros del juego
original: punteros NULL, lecturas del área de BIOS de la GBA y divisiones por cero. El juego
sigue y deja una línea `bios: ... at eip X` en `kh_com.log`. Android, la web y Linux ARM **no**
los recuperan: ahí el mismo acceso cierra el juego (Cloud/Axel/Parasite Cage: división por
cero; #30: escritura en NULL al acumular naipes).

Por eso cada línea `bios:` es un bug a arreglar en el código del juego, siguiendo el resultado
del cartucho: `BIOS_PTR(p)` para punteros que en la GBA caen en la BIOS, `DIV0`/`MOD0` para
divisiones que en la GBA dan 0, o una guarda con el mismo resultado.

## Uso

```
python port/tools/debug_kit.py run --build               # todo: 3 regiones, 6 baterías
python port/tools/debug_kit.py run --versions us --suites battles,events --quick
python port/tools/debug_kit.py report                    # solo leer resultados en disco
```

Baterías (todas con ventana oculta, sin audio, cada caso en su carpeta aislada):

| Batería | Qué hace |
|---|---|
| battles | cada combate del menú de depuración (`test_battles.py`) |
| riku | los mismos como Riku (Reverse/Rebirth) |
| events | cada evento de la historia (`test_events.py`) |
| wide | los mismos eventos en vista 16:9 (dibujo propio del port) |
| rooms | cada mundo y sala del castillo, caminando (`test_rooms.py`) |
| monkey | juego aleatorio reproducible desde cada piso (`test_monkey.py`) |

`--jobs` se elige solo según la RAM libre (~300 MB por proceso) para no dejar el PC sin
memoria mientras el usuario lo usa.

## Informe

`build/debugkit/report.md` y `report.json`: una fila por línea de código con el tipo de
acceso, cuántos casos la tocan y cuáles; los cierres y errores fatales aparte. Cada dirección
se simboliza con la copia del exe que corrió ese caso. "stale" = solo visto con un exe más
viejo que la build actual (repetir antes de arreglar).

## Después de un informe limpio

Android x86 con UBSan (desplazamientos, divisiones, NULL y accesos desalineados al logcat):

```
python port/tools/build_android.py --abi x86 --smoke --ubsan --smoke-env KHCOM_FAST=1
python port/tools/test_android_scenarios.py --serial emulator-5554 --regions us --groups menus fixtures bosses
```

Los avisos de accesos desalineados (p. ej. `ApproachValue`) no fallan en ARMv7/x86: no se
parchean. El emulador se cae si falta RAM: no compilar en paralelo.

## Historial

- 20/10: primera pasada. Arreglados antes del kit: `AllocObjAffine` (DIV0), `BosPcUpdateIdle`
  (MOD0), `MapPlaceDoorOnPlatform` (MOD0), `ClearStockedCardSlots` (#30).
