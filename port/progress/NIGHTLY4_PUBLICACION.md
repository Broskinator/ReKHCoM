# Nightly 4 — registro de entrega, 18/10/2026 (Claude)

Versión `v1.9.3-nightly.20261018.4`, prerelease pedida por el usuario
(«ir lanzando release con las funciones nuevas ya listas»). Stable sigue v1.9.2.

## Fuente

Tag en `880ae3e2d`. Windows y Linux/Steam Deck/Anbernic se compilaron desde
`6a541395c`; `880ae3e2d` solo añade stubs de estados y oculta la revancha en
Android/Web (código bajo `__ANDROID__`/`__EMSCRIPTEN__`), con los que se
compilaron el APK y la web.

Incluye: trucos (2fdca4ca7), auditoría de opciones (fbaa9ef16), editor de
partidas del launcher (b7d491ef5), aviso moguri (77e2eac4f), revancha de jefes
y tutoriales (8ebdb493e), #16 de Codex (4287b8053), diagrama de mando (b3a0f1e7d).
No incluye: cierre superior #17 (apagado por defecto), #18, Darkside WIP.

## Comprobaciones

- Windows EU/US/JP: paquete descomprimido, ejecutables de las tres regiones
  entran a sala, aplican trucos y aviso moguri, sin crash.
- Launcher 11/11 (test_launcher.py), test_settings_keys.py PASS,
  test_save_editor.py PASS, test_door_views.py 15/15.
- Paquetes revisados: sin ROM/SRAM/savestates; único ini el preset de Steam Deck.
- Android: APK ARMv7 con las tres regiones, versionCode 1090304, firma debug,
  no testOnly; sin prueba en dispositivo. Revancha oculta (sin estados).
- Web: publish_web.py valida Wasm/JS y manifiesto de fuente limpia.
- PS5: no recompilado en esta nightly (requiere host Linux nativo).
- Sin validación física de Odin 2, Anbernic, Steam Deck ni mando real.

## Publicación

- [Prerelease](https://github.com/MatyMeatBoy/ReKHCoM/releases/tag/v1.9.3-nightly.20261018.4):
  seis paquetes y SHA256SUMS-nightly4.txt; tamaños y SHA-256 de GitHub
  coinciden con los locales. API latest sigue en v1.9.2.
- Web: gh-pages `316bd8cf1`; el manifiesto servido es
  `v1.9.3-nightly.20261018.4-web-alpha`.

| Archivo | SHA-256 |
|---|---|
| Windows x86 ZIP | `e64f5c93ed0ecbbb053c1dfb75b6cfd7a80f970ffafcd837688bb61f6d98514a` |
| Linux x86 tar.gz | `35e1f3969c70316b23308a0e34150ef7be14771afeff1687a4a05c6483453307` |
| Steam Deck tar.gz | `ea8baeaf8d342134881dcd90fbac4d2237fd772b97eb2cc782b82cb65163473b` |
| Anbernic alpha ZIP | `b06f34b2b2e575dab5a7a1b51af35c8520cbc20c11da803e26475ecb068625ad` |
| Android alpha APK | `4264cac9e80c7385139ad0b0573161254077777fc93881f9451f026ab89fb472` |
| Web alpha ZIP | `377489c181003cc3247edfa8dc74f9ed83dec5c72895b6675ba8c68e0a481e6d` |
