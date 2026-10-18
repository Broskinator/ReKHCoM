# Toolkit de compilación y espacio (18/10/2026)

## Una sola VM para las consolas

La imagen `D:/ReKHCoM-build/ps5/ps5env.qcow2` (Debian 12 x86_64, con red) es la VM de
compilación de consolas. Se arranca con `port/tools/linux_vm_test.py --disk <imagen> --net`
(la carpeta compartida viaja como tar). Debe servir para todas: PS5 (ya instalado),
Vita (VitaSDK + vitaGL), PS4 (OpenOrbis) y Switch (devkitPro). Cada SDK va en su carpeta
propia bajo `/root/consoles/<nombre>` y la caché de descargas bajo `/root/consoles/.deps`,
para no duplicar sistema base. Las imágenes de prueba Linux (debian-12-nocloud-*) siguen
aparte: son de solo lectura (snapshot) y no crecen.

- PS5: `python port/tools/build_ps5_vm.py` (también lo llama `build_all.py --platform all
  --package` en Windows). Compila desde HEAD limpio; tarda más de media hora.
- Vita/PS4/Switch: mismo patrón, un `build_<consola>_vm.py` por consola (por crear).

## Reglas de espacio

- `python port/tools/clean_workspace.py` informa (y con `--apply` borra) lo que sobra en
  `build/`: evidencia, pruebas y registros; conserva cachés de compilación, releases,
  `userstates` (savestates del usuario) y `evidence`. Pasarlo a menudo.
- `--installs --keep-installs 3` poda las carpetas ReKHCoM-vX antiguas (respaldar guardados
  antes; solo cuando el usuario lo pida).
- No guardar ROM ni guardados dentro de `build/` salvo `userstates`.
- Worktrees viejos (`khcom`, `khcom-claude`, `khcom-linux`, `khcom-fix`, `khcom-codex-perf`,
  `khcom-wide-review`, `khcom-ps5`): su `build/` se puede vaciar; el código sin commitear
  de `khcom` y `khcom-claude` es WIP viejo: confirmar con el usuario antes de borrar el worktree.
