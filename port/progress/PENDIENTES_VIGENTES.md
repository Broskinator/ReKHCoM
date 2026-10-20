# Cola vigente (20/10/2026, 15:30)

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
| **Claude** | 51–54 PS4: alpha hecha (20/10, `port/ps4`): host OpenOrbis para la misma imagen x32 de PS5, .pkg desde Windows; probado el host en Linux (VM) hasta el menú, nunca en consola. Entra en el próximo prerelease |
| **Claude** | Switch (pedido por el usuario 20/10): imagen AArch64 de punteros de 32 bits y host en `port/switch` (commit c2d8b705a). Arranca en la VM ARM64 pero las listas de punteros de la ROM (`port/data/rom_pointers_*.inc`) siguen vacías: `build_gba_maps_vm.py` falla porque la ROM US compilada ya no coincide con el SHA1 del cartucho (algún cambio en `src/` rompió la build idéntica de GBA): buscarlo primero. Vita después, con el mismo modo reubicable |
| **Usuario** | Aprobar el launcher nuevo (`Probar launcher nuevo.bat`, commit 07a6fa9ab) |
| **Claude** | Release v1.9.11: candidato listo en main; se publica solo cuando el usuario diga |
| **Claude** | Auditoría de fallos (#30) con el kit de depuración (`debug_kit.py`, ver KIT_DEPURACION.md). Hecho: 65 jefes en Android x86 con UBSan, 209/209 combates y 52/52 salas US en escritorio sin fallos; arreglados Cloud/Axel/Parasite Cage y el editor de baraja (DIV0). En curso (20/10): Riku, eventos, eventos 16:9 y juego aleatorio US, luego EU y JP |
| **Claude** | Prerelease v1.9.11-pre1 publicado (20/10, solo Windows). Siguiente: stable v1.9.11 con todas las plataformas solo con OK del usuario |
| **Claude** | #30 en ARM físico: verificado solo en emulador x86; pedir logcat si sigue en el equipo de quien lo reportó |
| **Usuario** | Revisar 61–62 (escenas de Ciudad de Paso con márgenes incompletos) cuando Claude las muestre |
| **ChatGPT/Codex** | 39: rutas completas de Sora y de Reverse/Rebirth |
| **ChatGPT/Codex** | 16–24: arte de jefes y escenas, de a uno con antes/después |

## Vita y Switch (20/10)

El núcleo necesita punteros de 32 bits y las regiones GBA en sus direcciones reales
(0x02000000-0x0E010000). PS5/PS4 lo resuelven con la imagen x32 mapeada abajo; en Vita
(ARM32, espacio de usuario desde 0x81000000) y Switch (AArch64, sin 32 bits para homebrew,
región de código desde 0x08000000) no se puede mapear ahí. Hace falta reubicar todo el espacio
GBA (macros de hardware, enlazado, punteros de recursos, manejador de fallos ARM): proyecto
propio de semanas, que serviría para las dos. Alternativa Vita: ggdrn/KH_COM_VITA (MIT) ya lo
hizo con su propio parche.

## Esperando a terceros (no se trabaja)

- #23 mandos Nintendo: opción y normalización listas en juego y launcher; falta reporte con mando real (Switch Pro, 8BitDo, SN30).
- Issue #27 (scanlines en márgenes): abierto hasta que quien lo reportó confirme.
- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
