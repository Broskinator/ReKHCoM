# ENCARGO ACTIVO PARA CLAUDE — cerrar pendientes y publicar

Autorización del usuario, 18/10/2026: «recuerda terminar todos los pendientes y
publicar». Esta es una asignación concreta, no una propuesta opcional.

## Claude: tomar ahora estos encargos y devolver commits

1. **Guardado en cristal:** corregir resumen al cambiar slot, conservar último
   slot guardado al regresar al cristal y entre arranques. Corregir persistencia
   de la activación y revisar otras opciones afectadas. Mantener en Experimental.
2. **Savestate de borde superior abierto:** localizar el estado actual del usuario,
   respaldarlo y cerrar el mapa con sus propios tiles de borde.
3. **Savestate 3, Hollow Bastion:** completar checkerboard lateral mostrado en
   codex-clipboard-ed1ec70b-e50d-408d-8183-272a7d26d115.png. Conservar título y objetos.
4. **Integración y publicación:** recoger las correcciones de Codex de escenas
   057/061/074 cuando estén listas, integrar los puntos anteriores, comprobar
   EU/US/JP y publicar la entrega corregida. El usuario ya autorizó publicar.
   Registrar tag, enlace y plataformas realmente actualizadas.

Claude puede repartir 1/2/3 en workers distintos. Un integrador controla PPU.
Estos workers todavía no están confirmados como iniciados desde este chat.

## Codex de este chat: tarea en ejecución

**Corregir escenas 057 playa, 061 hielo y 074 Monstro fuera del encuadre antes
mostrado, usando resolución interna 960×540 de diagnóstico y vistas amplias.**
Conservar tiles originales y motivos completos, mostrar antes/después, preparar
commit para integración. León 004 permanece aprobada. Las otras tres son mejorables.
No asignar simultáneamente esas mismas escenas a otro worker.

## Cierre requerido

- [ ] Guardados: resumen, escritura aislada y último slot correctos.
- [ ] Activaciones persistentes, opción de slots en Experimental.
- [ ] Cierre superior del mapa del savestate.
- [ ] Checkerboard del savestate 3 completo.
- [ ] 057/061/074 corregidas y revisadas con vista amplia.
- [ ] Integración, comprobaciones regionales y publicación.

Los demás pendientes generales (jefes restantes, RetroAchievements, plataformas)
siguen registrados abajo y en CODEX_TAREAS.md; no darlos por terminados por cerrar
esta entrega. Los bugs nuevos de este bloque aún están pendientes.

---

# Repartija vigente para Claude y Codex — 18/10/2026

Este documento sustituye los estados anteriores que dicen «4/44 aprobadas».
El usuario pide coordinar la ayuda de Claude. Los workers de abajo son una
asignación propuesta; este chat no ha lanzado workers ni enviado mensajes.

## Prioridades y reparto

| Responsable propuesto | Encargo | Entrega necesaria |
| --- | --- | --- |
| Claude / worker guardados | Corregir selección de slot en el cristal y persistencia de opciones. | Resumen del slot correcto al cambiarlo, recordar último slot guardado al volver al cristal y tras reiniciar; opción en Experimental, conservar activación. |
| Codex de este chat / worker escenas | Revisar 057 playa, 061 hielo y 074 Monstro con una vista mucho más amplia. Pausar nuevas escenas. | Antes/después completos, misma cámara, zonas nuevas visibles; corregir motivos y bordes fuera del encuadre anteriormente aprobado. |
| Claude / worker mapas | Localizar el savestate del borde superior abierto y el savestate 3 del checkerboard de Hollow Bastion. | Reconstruir cierre superior con tiles del propio mapa; completar textura de ajedrez sin cortes de fase. Capturas completas y detalle. |
| Claude / worker jefes | Continuar auditoría de escenarios de combate tras Guard Armor. | Darkside siguiente; mantener inventario 1/8, sin duplicar jefe ni alterar fases. |
| Claude integrador | Recoger commits separados, resolver PPU compartido y preparar siguiente entrega. | Estado real de integración y límites de revisión; no promover las escenas mejorables a aprobación definitiva. |

Los responsables son el reparto solicitado, no confirmación de ejecución.
Claude puede reorganizar los workers; evitar dos workers editando port_ppu.c
simultáneamente. Preferir perfiles/headers por escena y un integrador del PPU.

## Peticiones nuevas del usuario: guardados

1. Al cambiar de slot, la pantalla no muestra correctamente lo que se va a
   guardar; consultar un slot que ya tenía partida sí muestra información.
   Investigar la actualización del resumen al mover la selección, antes y
   después de escribir; no confundir los datos existentes con la partida actual.
2. La función se desactiva al cerrar el juego. Mantener el valor de esta y
   las demás opciones que deban persistir; auditar lectura/escritura y launcher.
3. Mantener «seleccionar slot en puntos de guardado» dentro de Experimental.
4. Recordar el último slot realmente guardado al regresar al cristal.
   Persistir la selección para el siguiente arranque, sin sobrescribir otras ranuras.
5. Reportes del usuario pendientes de reproducir/corregir. Las pruebas antiguas
   de menú/configuración no demuestran que este flujo de gameplay esté resuelto.

Código de referencia en khcom-codex: port/src/port_config.c,
port/src/port_ui.c, port/decompui/decompui.cpp y src/map/map.c.
Clave actual: experimental_save_point_slot_select (default OFF).

## Estado correcto de escenas

**1/44 aprobada definitiva: 004 León. Otras 3/44 trabajadas, mejorables.**

| Evento | Estado vigente | Avance conservado |
| --- | --- | --- |
| 004 León | Aprobada explícitamente por el usuario | Conservar distribución. |
| 061 hielo/cristal | Mejorable; revisión amplia pendiente | Perfil por capas, esquina superior, rombo de suelo y panel sin sombra extraña. |
| 074 Monstro | Mejorable; revisión amplia pendiente | Paleta activa, tres capas, adorno verde completado con su otra mitad. |
| 057 playa | Mejorable; revisión amplia pendiente | Pared y arena; completar ambas mitades del pasto a izquierda y derecha. |

La aprobación anterior correspondía al encuadre mostrado; el usuario detectó
fallos al alejar zoom. No perder la distribución que sí funciona. Ampliar la
reconstrucción al área que antes quedaba fuera. No basta agrandar la imagen.
Propuesta de diagnóstico: lienzo interno 960×540 y una vista 32:9 amplia,
con varias cámaras, manteniendo cuadrícula y paleta nativas. No afirmar soporte
runtime de esas dimensiones hasta comprobar límites del renderizador.

Reglas acumuladas: partir de ROM y originales con fondo negro; no usar propuestas
fallidas como referencias. Reconstruir motivos enteros buscando su copia original,
incluida la otra mitad al otro lado de la unión. Resolver por capa, conservar
vacíos diseñados y usar paleta/efectos vivos. Mostrar escena completa antes/después,
área original y añadida, y contador. Congelar la base antes de mejorar.

Commits de este chat en khcom-claude, rama feature/title-art-16x9:
a3ff671f6, 18ef1948f, 8cf93bc78, a4427b269, c924ff625, b651e4b65.
Los perfiles 057/061/074 ya fueron conectados realmente al PPU en khcom-codex:
d606d69ca. Las recetas originales de este chat eran previews y headers separados.
Evidencia histórica: NIGHTLY2_INTEGRATION.md, 23/23 escenas y 13/13 Guard Armor;
no certifica todos los zooms/cámaras ni invalida los defectos reportados ahora.

## Savestates y referencia nueva

- Hay un estado del usuario con el borde superior del mapa abierto: localizarlo
  visualmente antes de elegir tiles. Cerrar según la disposición propia del mapa.
- Savestate 3: Hollow Bastion, faltan bandas de textura checkerboard a los lados
  del título/cartel. Completar patrón y fase, sin repetir el cartel ni los objetos.
- Imagen del reporte: C:/Users/MP/AppData/Local/Temp/codex-clipboard-ed1ec70b-e50d-408d-8183-272a7d26d115.png.
- Localizar los estados más recientes del usuario y respaldarlos junto con su
  ejecutable compatible antes de trabajar. No asumir que cualquier slot3 antiguo
  corresponde al reporte actual; no modificar estados ni partidas originales.

## Otros avances que Claude debe conservar

- Guard Armor aceptado e integrado: **1/8 mejoras de jefes**; quedan Darkside,
  Oogie, Jafar, Maléfica dragón, Úrsula, Marluxia final y tablero inferior Alicia.
  Hook e interior de Monstro de combate se conservan según el usuario.
- 28/28 fondos de combate inspeccionados inicialmente; no equivale a todas las
  fases ni todas las campañas. Ver BATTLE_SCENERY_AUDIT.md y BATTLE_SCENERY_TECHNIQUE.md.
- Undub opcional desde ROM JP y remapeo de controles/gatillos implementados.
  Dificultad y respuestas de issues: CODEX_TAREAS.md y CODEX_RESPUESTAS.md.
- RetroAchievements: investigación disponible, integración jugable pendiente.
- Documentos antiguos discrepan sobre respuestas #12/#13 y publicación Nightly 2:
  verificar estado efectivo antes de afirmar publicación o repetir respuestas.
- Último documento local prepara v1.9.3-nightly.20261018.2; no inferir de ello
  promoción a estable ni publicación remota verificada por este chat.

## Coordinación y entrega

Trabajar con commits por tarea y preservar cambios ajenos. En khcom-claude hay
trabajo previo sin commitear en include/engine.h y port/src/port_ppu.c, además
de port_menu_layout.h y ESCENAS_16x9.md sin seguimiento: no absorberlo por accidente.
No versionar ROM, saves, savestates ni arte extraído; evidencia privada en build/.
Cada worker debe dejar archivos/commit, qué cambió, capturas y límites pendientes.
La revisión visual amplia solicitada sí requiere comprobar las vistas nuevas.
Este documento solo organiza el trabajo; los bugs nuevos aún no están corregidos.
