## Entrega de Codex disponible — 18/10/2026

**Escenas amplias listas en main: 0cb04a8dd** (fuente 643739f9b).
No es necesario reconstruir mis previews ni repetir el trabajo de tiles.
EU/US/JP compilan; vistas reales 960×540 y 1440×810; 004 idéntica.
Capturas finales en khcom-wide-review/build/wide-runtime-review (061),
wide-runtime-beach-final (057), wide-runtime-monstro-final (074).
**Claude: recoger este commit si tu integración aún no lo tiene, cerrar tus
workers de guardado/mapas y publicar el conjunto corregido.** El nuevo trabajo
Darkside/PPU sin commitear de khcom-codex permanece intacto y fuera de mi commit.
Lista única: PENDIENTES_VIGENTES.md. No marcar 057/061/074 aprobadas definitivas.

# ENCARGO ACTIVO PARA CLAUDE â€” cerrar pendientes y publicar

AutorizaciÃ³n del usuario, 18/10/2026: Â«recuerda terminar todos los pendientes y
publicarÂ». Esta es una asignaciÃ³n concreta, no una propuesta opcional.

## Claude: tomar ahora estos encargos y devolver commits

1. **Guardado en cristal:** corregir resumen al cambiar slot, conservar Ãºltimo
   slot guardado al regresar al cristal y entre arranques. Corregir persistencia
   de la activaciÃ³n y revisar otras opciones afectadas. Mantener en Experimental.
2. **Savestate de borde superior abierto:** localizar el estado actual del usuario,
   respaldarlo y cerrar el mapa con sus propios tiles de borde.
3. **Savestate 3, Hollow Bastion:** completar checkerboard lateral mostrado en
   codex-clipboard-ed1ec70b-e50d-408d-8183-272a7d26d115.png. Conservar tÃ­tulo y objetos.
4. **IntegraciÃ³n y publicaciÃ³n:** recoger las correcciones de Codex de escenas
   057/061/074 cuando estÃ©n listas, integrar los puntos anteriores, comprobar
   EU/US/JP y publicar la entrega corregida. El usuario ya autorizÃ³ publicar.
   Registrar tag, enlace y plataformas realmente actualizadas.

Claude puede repartir 1/2/3 en workers distintos. Un integrador controla PPU.
Estos workers todavÃ­a no estÃ¡n confirmados como iniciados desde este chat.

## Codex de este chat: tarea en ejecuciÃ³n

**Corregir escenas 057 playa, 061 hielo y 074 Monstro fuera del encuadre antes
mostrado, usando resoluciÃ³n interna 960Ã—540 de diagnÃ³stico y vistas amplias.**
Conservar tiles originales y motivos completos, mostrar antes/despuÃ©s, preparar
commit para integraciÃ³n. LeÃ³n 004 permanece aprobada. Las otras tres son mejorables.
No asignar simultÃ¡neamente esas mismas escenas a otro worker.

## Cierre requerido

- [ ] Guardados: resumen, escritura aislada y Ãºltimo slot correctos.
- [ ] Activaciones persistentes, opciÃ³n de slots en Experimental.
- [ ] Cierre superior del mapa del savestate.
- [ ] Checkerboard del savestate 3 completo.
- [ ] 057/061/074 corregidas y revisadas con vista amplia.
- [ ] IntegraciÃ³n, comprobaciones regionales y publicaciÃ³n.

Los demÃ¡s pendientes generales (jefes restantes, RetroAchievements, plataformas)
siguen registrados abajo y en CODEX_TAREAS.md; no darlos por terminados por cerrar
esta entrega. Los bugs nuevos de este bloque aÃºn estÃ¡n pendientes.

---

# Repartija vigente para Claude y Codex â€” 18/10/2026

Este documento sustituye los estados anteriores que dicen Â«4/44 aprobadasÂ».
El usuario pide coordinar la ayuda de Claude. Los workers de abajo son una
asignaciÃ³n propuesta; este chat no ha lanzado workers ni enviado mensajes.

## Prioridades y reparto

| Responsable propuesto | Encargo | Entrega necesaria |
| --- | --- | --- |
| Claude / worker guardados | Corregir selecciÃ³n de slot en el cristal y persistencia de opciones. | Resumen del slot correcto al cambiarlo, recordar Ãºltimo slot guardado al volver al cristal y tras reiniciar; opciÃ³n en Experimental, conservar activaciÃ³n. |
| Codex de este chat / worker escenas | Revisar 057 playa, 061 hielo y 074 Monstro con una vista mucho mÃ¡s amplia. Pausar nuevas escenas. | Antes/despuÃ©s completos, misma cÃ¡mara, zonas nuevas visibles; corregir motivos y bordes fuera del encuadre anteriormente aprobado. |
| Claude / worker mapas | Localizar el savestate del borde superior abierto y el savestate 3 del checkerboard de Hollow Bastion. | Reconstruir cierre superior con tiles del propio mapa; completar textura de ajedrez sin cortes de fase. Capturas completas y detalle. |
| Claude / worker jefes | Continuar auditorÃ­a de escenarios de combate tras Guard Armor. | Darkside siguiente; mantener inventario 1/8, sin duplicar jefe ni alterar fases. |
| Claude integrador | Recoger commits separados, resolver PPU compartido y preparar siguiente entrega. | Estado real de integraciÃ³n y lÃ­mites de revisiÃ³n; no promover las escenas mejorables a aprobaciÃ³n definitiva. |

Los responsables son el reparto solicitado, no confirmaciÃ³n de ejecuciÃ³n.
Claude puede reorganizar los workers; evitar dos workers editando port_ppu.c
simultÃ¡neamente. Preferir perfiles/headers por escena y un integrador del PPU.

## Peticiones nuevas del usuario: guardados

1. Al cambiar de slot, la pantalla no muestra correctamente lo que se va a
   guardar; consultar un slot que ya tenÃ­a partida sÃ­ muestra informaciÃ³n.
   Investigar la actualizaciÃ³n del resumen al mover la selecciÃ³n, antes y
   despuÃ©s de escribir; no confundir los datos existentes con la partida actual.
2. La funciÃ³n se desactiva al cerrar el juego. Mantener el valor de esta y
   las demÃ¡s opciones que deban persistir; auditar lectura/escritura y launcher.
3. Mantener Â«seleccionar slot en puntos de guardadoÂ» dentro de Experimental.
4. Recordar el Ãºltimo slot realmente guardado al regresar al cristal.
   Persistir la selecciÃ³n para el siguiente arranque, sin sobrescribir otras ranuras.
5. Reportes del usuario pendientes de reproducir/corregir. Las pruebas antiguas
   de menÃº/configuraciÃ³n no demuestran que este flujo de gameplay estÃ© resuelto.

CÃ³digo de referencia en khcom-codex: port/src/port_config.c,
port/src/port_ui.c, port/decompui/decompui.cpp y src/map/map.c.
Clave actual: experimental_save_point_slot_select (default OFF).

## Estado correcto de escenas

**1/44 aprobada definitiva: 004 LeÃ³n. Otras 3/44 trabajadas, mejorables.**

| Evento | Estado vigente | Avance conservado |
| --- | --- | --- |
| 004 LeÃ³n | Aprobada explÃ­citamente por el usuario | Conservar distribuciÃ³n. |
| 061 hielo/cristal | Mejorable; revisiÃ³n amplia pendiente | Perfil por capas, esquina superior, rombo de suelo y panel sin sombra extraÃ±a. |
| 074 Monstro | Mejorable; revisiÃ³n amplia pendiente | Paleta activa, tres capas, adorno verde completado con su otra mitad. |
| 057 playa | Mejorable; revisiÃ³n amplia pendiente | Pared y arena; completar ambas mitades del pasto a izquierda y derecha. |

La aprobaciÃ³n anterior correspondÃ­a al encuadre mostrado; el usuario detectÃ³
fallos al alejar zoom. No perder la distribuciÃ³n que sÃ­ funciona. Ampliar la
reconstrucciÃ³n al Ã¡rea que antes quedaba fuera. No basta agrandar la imagen.
Propuesta de diagnÃ³stico: lienzo interno 960Ã—540 y una vista 32:9 amplia,
con varias cÃ¡maras, manteniendo cuadrÃ­cula y paleta nativas. No afirmar soporte
runtime de esas dimensiones hasta comprobar lÃ­mites del renderizador.

Reglas acumuladas: partir de ROM y originales con fondo negro; no usar propuestas
fallidas como referencias. Reconstruir motivos enteros buscando su copia original,
incluida la otra mitad al otro lado de la uniÃ³n. Resolver por capa, conservar
vacÃ­os diseÃ±ados y usar paleta/efectos vivos. Mostrar escena completa antes/despuÃ©s,
Ã¡rea original y aÃ±adida, y contador. Congelar la base antes de mejorar.

Commits de este chat en khcom-claude, rama feature/title-art-16x9:
a3ff671f6, 18ef1948f, 8cf93bc78, a4427b269, c924ff625, b651e4b65.
Los perfiles 057/061/074 ya fueron conectados realmente al PPU en khcom-codex:
d606d69ca. Las recetas originales de este chat eran previews y headers separados.
Evidencia histÃ³rica: NIGHTLY2_INTEGRATION.md, 23/23 escenas y 13/13 Guard Armor;
no certifica todos los zooms/cÃ¡maras ni invalida los defectos reportados ahora.

## Savestates y referencia nueva

- Hay un estado del usuario con el borde superior del mapa abierto: localizarlo
  visualmente antes de elegir tiles. Cerrar segÃºn la disposiciÃ³n propia del mapa.
- Savestate 3: Hollow Bastion, faltan bandas de textura checkerboard a los lados
  del tÃ­tulo/cartel. Completar patrÃ³n y fase, sin repetir el cartel ni los objetos.
- Imagen del reporte: C:/Users/MP/AppData/Local/Temp/codex-clipboard-ed1ec70b-e50d-408d-8183-272a7d26d115.png.
- Localizar los estados mÃ¡s recientes del usuario y respaldarlos junto con su
  ejecutable compatible antes de trabajar. No asumir que cualquier slot3 antiguo
  corresponde al reporte actual; no modificar estados ni partidas originales.

## Otros avances que Claude debe conservar

- Guard Armor aceptado e integrado: **1/8 mejoras de jefes**; quedan Darkside,
  Oogie, Jafar, MalÃ©fica dragÃ³n, Ãšrsula, Marluxia final y tablero inferior Alicia.
  Hook e interior de Monstro de combate se conservan segÃºn el usuario.
- 28/28 fondos de combate inspeccionados inicialmente; no equivale a todas las
  fases ni todas las campaÃ±as. Ver BATTLE_SCENERY_AUDIT.md y BATTLE_SCENERY_TECHNIQUE.md.
- Undub opcional desde ROM JP y remapeo de controles/gatillos implementados.
  Dificultad y respuestas de issues: CODEX_TAREAS.md y CODEX_RESPUESTAS.md.
- RetroAchievements: investigaciÃ³n disponible, integraciÃ³n jugable pendiente.
- Documentos antiguos discrepan sobre respuestas #12/#13 y publicaciÃ³n Nightly 2:
  verificar estado efectivo antes de afirmar publicaciÃ³n o repetir respuestas.
- Ãšltimo documento local prepara v1.9.3-nightly.20261018.2; no inferir de ello
  promociÃ³n a estable ni publicaciÃ³n remota verificada por este chat.

## CoordinaciÃ³n y entrega

Trabajar con commits por tarea y preservar cambios ajenos. En khcom-claude hay
trabajo previo sin commitear en include/engine.h y port/src/port_ppu.c, ademÃ¡s
de port_menu_layout.h y ESCENAS_16x9.md sin seguimiento: no absorberlo por accidente.
No versionar ROM, saves, savestates ni arte extraÃ­do; evidencia privada en build/.
Cada worker debe dejar archivos/commit, quÃ© cambiÃ³, capturas y lÃ­mites pendientes.
La revisiÃ³n visual amplia solicitada sÃ­ requiere comprobar las vistas nuevas.
Este documento solo organiza el trabajo; los bugs nuevos aÃºn no estÃ¡n corregidos.

## VerificaciÃ³n del integrador de main â€” 18/10/2026

Nightly 2 publicada y verificada por GitHub API/CLI como pre-release:
https://github.com/MatyMeatBoy/ReKHCoM/releases/tag/nightly-2026.10.18.2
Tag sobre `d606d69ca`. SHA-256 ZIP:
`8ca3ea7ca7bc9cf2dc57fa4f7609ad0919727567a38e8ebd90bacdf4716b4691`.
La estable Latest sigue siendo v1.9.2. La publicaciÃ³n no convierte la revisiÃ³n
inicial de 057/061/074 en aprobaciÃ³n definitiva de todas las cÃ¡maras. Se adopta
el estado actualizado: 004 definitiva; las otras tres mejorables. El chat de
jefes continÃºa Darkside por instrucciÃ³n directa del usuario, sin lanzar workers.
