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
| **Claude** | Auditoría de accesos NULL y divisiones por cero en todo el juego (#30): `test_battles.py` EU/US/JP + Riku con las builds de escritorio, simbolizar cada `bios:` y parchear con guarda o `BIOS_PTR` (Android, web y Linux ARM no recuperan el fallo); luego revisar por lógica fallos parecidos (lecturas de punteros abiertos, divisiones, desbordes) y probar en el emulador Android x86 |
| **Claude** | #23 mandos: falta opción `pad_layout` en Controles (juego y launcher) y probar con mando real (Switch Pro, 8BitDo, SN30); el launcher aún no normaliza botones |
| **Claude** | Traducciones de logros (es fr de it ja): integradas; falta revisión de calidad por idioma |
| **Claude** | Tests de logros desactualizados: `test_achievement_menu.py` (controles nuevos L/R y saltos de 5) y `test_achievement_presentation.py` (firma con insignia) |
| **Claude** | Prerelease v1.9.11 tras lo anterior: `release_check.py` antes y después, web publicada, stable solo con OK del usuario |
| **Claude** | #30 en ARM físico: verificado solo en emulador x86; pedir logcat si sigue en el equipo de quien lo reportó |
| **Usuario** | Revisar 61–62 (escenas de Ciudad de Paso con márgenes incompletos) cuando Claude las muestre |
| **ChatGPT/Codex** | 39: rutas completas de Sora y de Reverse/Rebirth |
| **ChatGPT/Codex** | 16–24: arte de jefes y escenas, de a uno con antes/después |

## Esperando a terceros (no se trabaja)

- Issue #27 (scanlines en márgenes): abierto hasta que quien lo reportó confirme.
- Quiver: issue tgeorgiadis/quiver-launcher#182, pedimos levantar la retención del
  repositorio (VirusTotal 0/68; la causa fueron releases borrados mientras los revisaban).
