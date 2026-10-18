# Nightly 5 — registro de entrega, 18/10/2026 (Claude)

Versión `v1.9.3-nightly.20261018.5`, prerelease pedida por el usuario («arma una nightly... para probar el 49»).
Stable sigue v1.9.2. Tag en `f00fa1140`; Windows y Linux se
compilaron desde `3696d3912` (+ `build_linux.py` corregido para Linux/Steam Deck/Anbernic).

Incluye: borde superior de Riku (#17), logros locales EU/US/JP, arte del mando con
resaltado (49), transición de Opciones sin salto de escala, fuente CJK de Linux,
corrección del paquete Linux (#19). Escenas 061/074 aprobadas, 057 con mejoras (Codex).
No incluye PS5 (requiere host Linux nativo).

## Comprobaciones

- Windows: paquete descomprimido, exe EU entra a la sala de Riku sin crash; sin ROM/SRAM.
- Linux/Steam Deck/Anbernic: los tres traen libasound.so, libX11.so, libpulse.so,
  libudev.so y libdrm.so (nombres sin versión que abre SDL).
- Web: publish_web.py validó Wasm/JS; manifiesto servido `v1.9.3-nightly.20261018.5-web-alpha`, gh-pages ef3d8eed8.
- Tamaños y SHA-256 de GitHub coinciden con los locales; latest de la API sigue v1.9.2.

| Archivo | SHA-256 |
|---|---|
| ReKHCoM-v1.9.3-nightly.20261018.5-anbernic-alpha.zip | `d1d7440f363576724d456e5bc12d85f32ec7c2cbc011a6931a5a838e31ca72a8` |
| ReKHCoM-v1.9.3-nightly.20261018.5-android-alpha.apk | `0852dee3f2c6de6e09a0890e04b1b61ab82dd4b235cc786fa6a97267e9457aa3` |
| ReKHCoM-v1.9.3-nightly.20261018.5-linux-x86.tar.gz | `3ccc04ac1e7096a235170b546ff58bc8cdb7b3bc8f15883409880b82e1e5f35b` |
| ReKHCoM-v1.9.3-nightly.20261018.5-steamdeck.tar.gz | `478565f93c6d2c33c56bb9c809094cc15ec8998d341ec22a98527dd64fc6e494` |
| ReKHCoM-v1.9.3-nightly.20261018.5-web-alpha.zip | `18dde7690c681135148ddfd7dc860fb498e5ef9eca9cbdc3553c6b8340493852` |
| ReKHCoM-v1.9.3-nightly.20261018.5-windows-x86.zip | `da828c32dd0407373ca40e0e1e0465c2d45a253934efffd0f5c98a940bdb4aa2` |
