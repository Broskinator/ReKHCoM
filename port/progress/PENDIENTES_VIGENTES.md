# Cola vigente (21/10/2026, reparto con Codex)

Publicado: v1.9.12 estable (Latest) con Windows, Linux, Steam Deck, Anbernic, Android, PS5, PS4 y ahora las alphas de Switch y Vita; web en Pages. Issues #23, #24, #30 y #31 cerrados; #25 abierto. PR #32 integrado.

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
| **Claude** | Discord Rich Presence (c7326c6a4, 584f8315a): hecho y probado con un Discord falso (`fake_discord_ipc.py`); falta el Application ID de Discord que crea el usuario (`Probar Discord.bat`) y fijarlo en `REKHCOM_DISCORD_APP_ID`; luego probar Linux |
| **Claude** | Aviso de SmartScreen al abrir `khcom_eu.exe` (pedido del usuario 21/10): opciones explicadas, espera decisión (firma de código o que el launcher desbloquee sus propios .exe) |
| **Claude** | Auditoría de fallos (#30) con el kit de depuración (`debug_kit.py`, KIT_DEPURACION.md): sigue con Riku, eventos, eventos 16:9 y juego aleatorio US, luego EU y JP |
| **Claude** | Reportes de hardware de Switch/Vita cuando lleguen; meterlas en la próxima release (stage_release ya las incluye) |
| **ChatGPT/Codex** | NUEVO (pedido del usuario 21/10): opción "Volver a la pantalla de título" en cualquier momento, en la sección Re:CoM de Opciones (in-game `port_ui.c`, launcher `decompui.cpp`, `kh_com.ini`), justo encima de "Devolver cartas acumuladas" (`ITEM_UNSTOCK`). Por la lógica del juego (reinicio suave), no objetos del port. Copia jugable + .bat para aprobar |
| **ChatGPT/Codex** | 39: rutas completas de Sora y de Reverse/Rebirth |
| **ChatGPT/Codex** | 16–24: arte de jefes y escenas, de a uno con antes/después |
| **Usuario** | Crear la aplicación de Discord y pasar su Application ID; probar `Probar Discord.bat` y `Probar ReKHCoM v1.9.13 pre1.bat` (arranque en pantalla completa sin parpadeo, df56a0a2d) |
| **Usuario** | Aprobar el launcher nuevo (`Probar launcher nuevo.bat`, commit 07a6fa9ab); revisar 61–62 cuando Claude las muestre |

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
- Build idéntica de GBA recuperada (7c7fb0f09, 62777ed3f): ROM EU/US/JP iguales al cartucho.
- Listas de punteros de EU/US/JP (`gen_rom_pointers.py`, 132037/98603/98035 palabras), cada una
  comprobada contra el cartucho; incluyen las desalineadas (GOTO/PATT de las canciones).
- Switch (`port/switch`, README): imagen AArch64 de punteros de 32 bits para Cortex-A57 (sin
  `-mcpu` clang emitía `xpaci` de ARMv8.3 y caía), memcpy/memset de 16 bytes, `.nro` de las tres
  regiones (`build_switch.py host`, devkitA64 de la imagen Docker devkitpro/devkita64 porque
  apt.devkitpro.org responde 403).
- Vita (`port/vita`, README): misma imagen reubicable en ARMv7 (clang, Cortex-A9), aislada en un
  objeto con solo `guest_main`/`gHostTable` globales y enlazada con el host compartido
  (`switch_host.c` con `__vita__`: rasterizador a 960x544, sceAudioOut, sceCtrl); un ejecutable por
  región (eboot.bin EU, eboot_us/jp.self). VitaSDK en la VM de consolas. `build_vita.py test` corre la
  misma imagen en Linux ARM de 32 bits.
- Verificado en la VM ARM64: título, menús y la batalla KHCOM_BATTLE=10:4 en Switch y en Vita; el
  fotograma 1490 sale idéntico píxel a píxel en las dos imágenes.

Sin verificar (no hay consola ni emulador aquí): mapeo de memoria de código en Switch real, pantalla,
audio y mandos del host de Vita, rendimiento. Llega como reporte de quien lo pruebe.

Siguiente:
1. Switch: PUBLICADA como alpha dentro de la release v1.9.12 (`ReKHCoM-v1.9.12-switch-alpha.nro`,
   pedido del usuario 20/10; archivos viejos intactos). La próxima release la lleva desde stage_release.
   Vita: PUBLICADA igual (`ReKHCoM-v1.9.12-vita-alpha.vpk`, 20/10).

## Esperando a terceros (no se trabaja)

- #23 mandos Nintendo: opción y normalización listas en juego y launcher; falta reporte con mando real (Switch Pro, 8BitDo, SN30).
- Issue #27 (scanlines en márgenes): abierto hasta que quien lo reportó confirme.
- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
