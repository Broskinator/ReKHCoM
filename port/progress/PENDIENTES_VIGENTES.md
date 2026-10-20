# Cola vigente (20/10/2026, 17:00)

Publicado: v1.9.12 estable (Latest) con Windows, Linux, Steam Deck, Anbernic, Android, PS5 y PS4; web en Pages. Issues #23, #24, #30 y #31 cerrados; #25 abierto. PR #32 integrado.

Única cola de trabajo; los demás .md son evidencia. Lo terminado sale de aquí
(queda en el historial de git y en CHANGELOG.md). Numeración histórica conservada.

Reglas del usuario:
- Claude y Codex commitean y pushean a main; si uno lleva más de 10 min sin commits ni
  archivos tocados, el otro toma sus puntos anotando hora y evidencia aquí.
- Todo lo visual se entrega como copia jugable + .bat y espera aprobación del usuario; lo
  aprobado va a LOGICA_APROBADOS.md; método en PRUEBAS_VISUALES.md.
- La validación en hardware real no es pendiente: llega como reporte.
- Nunca borrar carpetas ReKHCoM-vX sin que el usuario lo pida.

## Reparto

| Quién | Pendiente |
|---|---|
| **Claude** | Switch y Vita (pedido del usuario, 20/10): estado y pasos en "Vita y Switch" abajo. Ahora: listas de punteros de la ROM de EU/US/JP en la VM, luego probar la imagen de Switch en la VM ARM64 |
| **Usuario** | Aprobar el launcher nuevo (`Probar launcher nuevo.bat`, commit 07a6fa9ab) |
| **Claude** | Auditoría de fallos (#30) con el kit de depuración (`debug_kit.py`, ver KIT_DEPURACION.md). Hecho: 65 jefes en Android x86 con UBSan, 209/209 combates y 52/52 salas US en escritorio sin fallos; arreglados Cloud/Axel/Parasite Cage y el editor de baraja (DIV0). En curso (20/10): Riku, eventos, eventos 16:9 y juego aleatorio US, luego EU y JP |
| **Usuario** | Revisar 61–62 (escenas de Ciudad de Paso con márgenes incompletos) cuando Claude las muestre |
| **ChatGPT/Codex** | 39: rutas completas de Sora y de Reverse/Rebirth |
| **ChatGPT/Codex** | 16–24: arte de jefes y escenas, de a uno con antes/después |

## Vita y Switch (20/10, 17:00)

Por qué es distinto de PC/PS5/PS4: el juego lee los datos del volcado del usuario, y esos datos
llevan punteros absolutos de GBA (0x08xxxxxx). PC, Android, web, PS5 y PS4 cargan la ROM en
0x08000000 y los punteros valen tal cual. Switch (AArch64, código desde 0x08000000, homebrew sin
procesos de 32 bits) y Vita (ARM32, usuario desde 0x81000000) no pueden usar esas direcciones.

Nuestro método (PORT_GBA_RELOC, `include/gba/gba_space.h`): todas las regiones GBA son arreglos del
programa; al cargar la ROM se corrigen sus punteros con la lista exacta del enlazador
(`port/data/rom_pointers_*.inc`, de reenlazar la ROM del decomp con `--emit-relocs`), y los
accesos sueltos a direcciones de bus se traducen (`GbaSpacePtr`). Una misma imagen sirve a las dos.
ggdrn/KH_COM_VITA (MIT) hace lo equivalente de otra forma: enlaza los datos dentro del ejecutable
(el enlazador de Vita corrige los punteros), los pone a cero y los rellena desde la ROM del usuario
al arrancar; los punteros dentro de bloques binarios los traduce con `GBA_PTR()` en cada uso.
Las dos vías necesitan que el decomp compile la ROM idéntica al cartucho.

Hecho:
- Modo reubicable: cabeceras, símbolos fijos (`rom_symbols_reloc.s`), build de comprobación en
  Windows (`configure_pc.py --reloc`) con el rango del bus reservado sin acceso.
- Switch: imagen del juego AArch64 con punteros de 32 bits (clang `arm64_32`, Mach-O, enlazada en
  0x10000000) y host libnx / host de prueba Linux AArch64 (`port/switch`, c2d8b705a). Arranca en la
  VM ARM64; sin las listas de punteros se cae al poco.
- Build idéntica de GBA recuperada (rota desde el 14/10): envoltorios de dígitos de barajas
  (7c7fb0f09), desbloqueo de Riku del título y ranura del cristal (62777ed3f) solo en PC. ROM US
  igual al cartucho (SHA1 10729bd8...). Herramientas: `build/gbacmp/` (vista GBA de las fuentes
  contra el decomp, buscador de desplazamientos).

Siguiente:
1. Listas de punteros de EU/US/JP (`build_gba_maps_vm.py`) y comprobar que el reenlace da la misma ROM.
2. Imagen de Switch con las listas, en la VM ARM64 hasta el menú y una partida; luego .nro.
3. Vita: host propio (vitasdk) sobre la misma imagen reubicable, ARM32.
4. Release: Switch y Vita como alphas en la misma release, solo cuando arranquen.

## Esperando a terceros (no se trabaja)

- #23 mandos Nintendo: opción y normalización listas en juego y launcher; falta reporte con mando real (Switch Pro, 8BitDo, SN30).
- Issue #27 (scanlines en márgenes): abierto hasta que quien lo reportó confirme.
- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
