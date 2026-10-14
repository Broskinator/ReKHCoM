# Codex retomó — 14/10/2026, 08:47 (Santiago)

Objetivo vigente: trabajar junto a Claude hasta verificar todas las barras
al 100 %. Claude mantiene integración y releases. No publicar ni sustituir
partidas/ejecutables del usuario desde este bloque.

## Reparto al retomar

- Claude está modificando `port_ppu.c`, `src/engine.c`, `engine.h` y
  `REPARTO.md` en `khcom-claude`: conserva el cierre de headers/transiciones
  de menús. Codex no toca esos archivos mientras esté ese trabajo activo.
- Codex retoma **Gameplay verification**: contraste del replay VBM USA con
  el cartucho, sincronía tras la intro y rutas reproducibles de ambas
  campañas. Trabaja en herramientas de replay y evidencia independiente.
- El árbol `khcom` está limpio en ef3a82f3b; los cambios de la sesión anterior
  ya fueron integrados por Claude. No reintroducir versiones antiguas.
- `status.json` solo deja abiertas menús/eventos (93 %) y gameplay (85 %).
  El cuadro antiguo de REPARTO y varios informes históricos conservan
  porcentajes anteriores; leer el estado actual y sus commits de evidencia.

## Lo que queda por demostrar en gameplay

El replay local USA de 20.000 frames terminó sin crash pero seguía en el
primer combate tutorial, nivel 1. Esto no demuestra una partida completa.
La cinta tiene 283.693 frames; antes de ejecutarla completa hay que contrastar
la misma secuencia en emulador y encontrar el primer punto de divergencia.
No inyectar progreso ni convertir batallas/eventos de debug en una campaña.

Las herramientas `test_gameplay_*` quedan como evidencia de salas, submenús,
pacing y replay; `test_rooms`, `test_submenus`, `test_ppu` y `test_options`
de Claude se conservan. Sus alcances son diferentes y no se borran por
duplicidad nominal.

Al terminar un bloque, Codex hace commit solo de sus archivos propios para
que Claude pueda integrarlo. Las capturas/ROMs/cintas/partidas permanecen en
directorios locales ignorados.
