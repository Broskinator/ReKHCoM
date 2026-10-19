# 71 RetroAchievements: vía rápida (propuesta, 19/10)

Hoy van 28/90 oficiales. Cada variable que lee el set oficial se proyecta a mano a su dirección
GBA en `port_ra_state.c` (por ejemplo `gPoohGauge` → 0x0203C9D4) y se valida una por una con
BizHawk. Es lento, porque los 62 restantes leen decenas de variables y cadenas de punteros.

## Idea

La decomp compila el ROM original byte a byte. Su mapa de enlace (`build/<v>/com_<v>.map`)
da la dirección GBA exacta de **cada** variable de IWRAM y EWRAM. El ejecutable del port
conserva los nombres de esas mismas variables, incluidas las `static`: 7141 símbolos de datos
en `khcom.exe` (`llvm-nm`: `_sTitleState`, `_gBtlWork`, `_gPoohGauge`…). Basta con
emparejarlas por nombre para proyectarlas todas de una vez, sin escribirlas a mano.

## Pasos

1. **Mapa GBA.** En la VM Debian (`ps5env.qcow2`, con red):
   `tools/setup_legacy_toolchain.py` (agbcc y binutils 2.10), luego `configure.py` y `ninja`
   para eu, us y jp. Se verifica el sha1 del ROM y se guardan los tres `.map` (no contienen
   datos del cartucho, solo direcciones).
2. **Tabla automática.** Un script posterior al enlace cruza `llvm-nm khcom.exe` con el
   `.map` y produce, por región, la lista (dirección del port, dirección GBA, tamaño) de todas
   las variables comunes.
   - El tamaño sale del `.map` (distancia al siguiente símbolo de la misma sección).
   - Si el tamaño del port difiere (por un campo `PLATFORM_PC`), la variable se excluye y se
     informa.
   - La tabla va como recurso junto al exe o enlazada en una sección propia.
3. **Proyección genérica.** `PortRaReadMemory` superpone todas las variables de la tabla.
   - Los punteros que guardan apuntan al heap del juego, que ya vive en las direcciones GBA
     (0x02000000–0x02034000), así que se copian tal cual.
   - Las palabras que apunten a otra variable del juego (sección del port 0x0EB0xxxx) se
     traducen con la misma tabla.
   - Los punteros a código o a datos del host se publican como 0, igual que hoy.
4. **Estructuras del heap.** `ra_layout_check.py` ya compara offsets. Con el mapa se puede
   automatizar: para cada `EwramAlloc` de una estructura que use el set oficial, comparar
   `sizeof` y offsets entre el port y el original (DWARF del port contra la decomp, que son el
   mismo código C).
5. **Validación masiva.**
   - `KHCOM_RA_DRYRUN` evalúa las 90 definiciones sin conexión.
   - Se repiten los escenarios ya automatizados (`bosskill`, salas, cartas) y los fixtures de
     RAM del original (BizHawk) que ya existen.
   - Se activan en `sOfficialVerified` todos los que salten justo en su condición y nunca en
     el control.

## Por qué es rápido

Los pasos 1 a 3 son mecánicos, se hacen una vez y cubren las tres regiones. Después, cada logro
solo necesita una prueba de "salta cuando debe y no antes" con las herramientas que ya tenemos.
Sin la tabla, cada logro obliga a descubrir sus direcciones a mano.

## Riesgos

- Estructuras con campos solo del port: se detectan en el paso 2 y quedan fuera, informadas.
- Algún entero que caiga por casualidad en el rango 0x0EB0xxxx se traduciría por error. Solo
  se traducen palabras alineadas dentro de campos de tipo puntero, según el DWARF del port.
