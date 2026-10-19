# 71 — avances de Codex, 19/10/2026

Base recibida de Claude: 21/90 oficiales activos en Windows/EU (softcore),
69 pendientes de validar; 16 retos locales en EU/US/JP. Esto reemplaza las
notas históricas del 27 que indicaban evaluación oficial totalmente apagada.
No se vuelven a aplicar los roots de batalla: ya están en 3a27f7aa9.

## Bloqueo de modificadores antes de evaluar

El frame nuevo ejecutaba KhRaPump antes de invalidar la sesión por un truco
activo. Eso permitía una evaluación oficial en el primer frame modificado.
Se invalida primero; apagar luego el truco no recupera premios en esa sesión.

Comprobación: test_official_frame_guard.py enlaza el port_achievements.c real
con un espía RA sin transporte ni credenciales. Los 12 casos EU/US/JP ×
one-hit/HP infinito/desbloqueo Riku/modo PS2 prueban frame limpio, primer frame
modificado y siguiente frame tras desactivar el truco. Ningún premio real ni
petición de red se genera. build/official-frame-guard.log.

## Trabajo que sigue

Validación de las cadenas de batalla/cartas, memoria fija del heap, Pooh,
piso e IWRAM contra el original. No se amplía sOfficialVerified por pasar un
mock o compilar. La ampliación requiere evidencia de los campos reales.
Fondos 61/62 y 16–24 apartados por el usuario. Commits de avances comprobables
aproximadamente cada cinco minutos (última instrucción, sustituye tres minutos).
