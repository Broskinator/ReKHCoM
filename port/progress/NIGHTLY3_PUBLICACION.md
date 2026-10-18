# Nightly 3 — registro de entrega, 18/10/2026

Versión: `v1.9.3-nightly.20261018.3`. Prerelease autorizada por el usuario.
Stable permanece `v1.9.2`. No se declara terminada la cola general.

## Fuente integrada

- Claude: `e9f053595` integrado como `aa8ef4434`; informe `49a523a09`
  integrado como `b7e9da465`. No se integró el borrador duplicado de Codex.
- Táctil persistente: `6d0f33df1`. Experimental → Ocultar controles táctiles
  en Android/Web, limpieza de pulsaciones y acceso de recuperación a Opciones.
- Build Linux/Web: `333b0f9a0`, enlace de soporte de voz portable.
- PS5: implementación de PR #15 y empaquetado ya integrados. El paquete se
  compiló desde `6d0f33df1`; cambios posteriores solo afectan build Linux/Web
  y documentación, no sus fuentes compiladas. Versión de app PS5: 1.9.3.
- Escenas previas incluidas; solo León tiene aprobación visual definitiva.
  Darkside WIP excluido. No se distribuyen ROMs ni datos personales.

## Comprobaciones realizadas

- Windows EU/US/JP: compilación y paquete; launcher 11/11.
- Configuración: 8/8 reinicios/combinaciones de selector y táctil, conservando
  también una clave desconocida. Selector Experimental off→on persistente.
- Táctil: multipulsación, diagonal, menús, ocultación/restauración, limpieza
  de pulsaciones y recuperación de Opciones correctos en harness de entrada.
- Web: runtime JS, controles/centrado en cinco proporciones y ABI Wasm 5/5.
- Linux i386 y ARMhf EU/US/JP: compilación y paquetes. Linux x86 EU/US/JP
  arrancan y alcanzan room/fieldfreeze en VM con SDL dummy; los tres salen 0.
- Android: tres regiones ARMv7, APK versionCode 1090303; verificación de libs
  incluidas. Firma debug de alpha, no testOnly.
- PS5: compilación EU/US/JP y ZIP íntegro. Transferencia recibida de 21.996.849
  bytes y SHA-256 contrastado con el calculado en la VM.
- Archivos comprimidos leídos y comprobados, sin ROM/SRAM/savestates ni INI
  personales; Steam Deck incluye únicamente su preset intencionado.

Evidencia privada bajo `build/nightly3-*` y manifiestos en `build/releases/`.
La prueba visual SDL software del fixture falla también sobre el código previo;
no se cuenta como prueba aprobada. No hay validación física Odin 2, Anbernic o
PS5; las plataformas alpha conservan esa condición. Pendientes de savestates
Hollow Bastion, revisión visual y pruebas físicas siguen en la lista única.

## Publicación

- [Prerelease publicada](https://github.com/MatyMeatBoy/ReKHCoM/releases/tag/v1.9.3-nightly.20261018.3).
- Tag/fuente de publicación: `568e43d5d0ed6b03e1c809108c947641b747e34a`.
- [Web alpha actualizada](https://matymeatboy.github.io/ReKHCoM/).
  gh-pages `3fe92a1e3ab7167b908c45180cd5caae944fa054`, deployment 39133622902
  completado. Manifest `v1.9.3-nightly.20261018.3-web-alpha`, fuente limpia.
  Los 19 archivos públicos descargados coinciden byte a byte con sus SHA-256.
- Siete paquetes, siete archivos SHA individuales y SHA256SUMS-nightly3.txt.
  Tamaños y digests SHA-256 de GitHub coinciden con los archivos locales.
- API latest confirma `v1.9.2`; Nightly 3 está marcada como prerelease.

| Archivo | SHA-256 |
|---|---|
| Windows x86 ZIP | `9186d1eb51dfd74aa8678446d86c4eb8bdf672ba912b70e5d7feec4722b9e9a2` |
| Linux x86 tar.gz | `e4e1e8f0691038c675e80dcd461749c14225ef626e53861f816dd0329eab460c` |
| Steam Deck tar.gz | `86b235cce972129bef42d19acc59687a1409b73cdd96af5ada42cbc4406c81c8` |
| Anbernic alpha ZIP | `d2313325286beac55e2f36f1481b46070e0fdc6141bf52c269d5f3bc48d771f7` |
| Android alpha APK | `f1c9ccceb63c85b7681318a56a55ef7b6d2148b1383e885388bfff4f60c00309` |
| Web alpha ZIP | `e178d41787b63d71924164a754f74e78ed6e2328cd9058f82503959e3efde13b` |
| PPSA98511.zip | `784774a76eeb7ba45626475670e42661e10580e4918c7c824928e1f1d9204278` |
