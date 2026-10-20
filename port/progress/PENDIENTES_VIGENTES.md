# Cola vigente (19/10/2026)

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
| **Claude** | 51–54: alphas de Vita, PS4 y Switch, compiladas como la de PS5 (VM) |
| **Claude** | Release v1.9.11: candidato listo en main; se publica solo cuando el usuario diga |
| **Claude** | Auditoría de accesos NULL y divisiones por cero (#30). Hecho: 65 jefes en Android x86 con UBSan (Cloud, Axel y Parasite Cage parchados con `DIV0`/`MOD0`, valen para todas las plataformas). En curso (20/10): `test_battles.py` de escritorio US/EU/JP + Riku con main, simbolizar cada `bios:` y parchear; luego eventos (`test_events.py`) |
| **Claude** | Traducciones de logros: español revisado (bien; nombres del cartucho, VT/PN/PA); falta fr, de, it, ja |
| **Claude** | Insignias de logros fuera de Windows/Linux: arreglado en main (`PortLoadAsset`, APK y web llevan el atlas); falta verlo en el emulador Android y en una build web cuando haya RAM libre. PS5: su carpeta de datos no trae el atlas, probar en la VM |
| **Claude** | Prerelease v1.9.11 tras lo anterior: `release_check.py` antes y después, web publicada, stable solo con OK del usuario |
| **Claude** | #30 en ARM físico: verificado solo en emulador x86; pedir logcat si sigue en el equipo de quien lo reportó |
| **Usuario** | Revisar 61–62 (escenas de Ciudad de Paso con márgenes incompletos) cuando Claude las muestre |
| **ChatGPT/Codex** | 39: rutas completas de Sora y de Reverse/Rebirth |
| **ChatGPT/Codex** | 16–24: arte de jefes y escenas, de a uno con antes/después |

## Esperando a terceros (no se trabaja)

- #23 mandos Nintendo: opción y normalización listas en juego y launcher; falta reporte con mando real (Switch Pro, 8BitDo, SN30).
- Issue #27 (scanlines en márgenes): abierto hasta que quien lo reportó confirme.
- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
