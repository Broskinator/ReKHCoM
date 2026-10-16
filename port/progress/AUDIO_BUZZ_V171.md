# Audio v1.7.1: reproducción, causa y prevención

## Punto de partida

El usuario confirmó **ranura 1 de v1.6.7**. Se conserva intacta en
ReKHCoM-v1.6.7/ReKHCoM-v1.6.7-windows-x86/states/slot1.khst. Claude había copiado
la misma ranura a build/fx_st. Se reprodujo con su ejecutable compatible en
build/audio-buzz-reference, sin escribir sobre partidas reales.

La escena es un combate de Ciudad de Paso. Se capturaron 900 fotogramas de mezcla
PCM float32 estéreo: bloques de 1.056 muestras a 63.072 Hz. La captura no presenta
clipping (aprox. -0,376 a +0,358). La reproducción y captura del punto están
confirmadas; la valoración auditiva final del zumbido corresponde a la prueba del
usuario. No confundir ese juicio con la demostración matemática siguiente.

## Defecto confirmado y corregido

MixChannel leía q[0] o *q como siguiente muestra incluso cuando count era 1.
En ese caso q apunta al byte posterior al sonido, que puede pertenecer a otro
recurso de la ROM. Ocurría tanto al preparar un bloque como al avanzar dentro
del mismo. La interpolación cúbica podía además usar loopStart[1] en un bucle de
una única muestra. El resultado dependía de datos que no eran sonido.

Ahora la siguiente muestra es la del bucle real o el último valor válido para
sonidos sin bucle. El vecino cúbico usa el índice modular del bucle, incluyendo
el caso de longitud 1. Se conserva el avance temporal, envolventes y mezcla.

Regresión ROM-free: muestras de longitud 1/2/3, con y sin bucle, tono fijo y
variable, interpolación lineal y suave. Se cambia deliberadamente el relleno
posterior de +96 a -96. El sonido debe ser idéntico y un bucle constante debe
mantener exactamente su valor. **Antes: 0/24; después: 24/24**.

## Cola del dispositivo

SDL_OpenAudioDeviceStream abre el dispositivo pausado, pero el código anterior
lo reanudaba antes de recibir la primera muestra. Aunque Queue calculaba una
meta de cuatro fotogramas, ese margen inicial no existía: la reproducción ya
competía con la producción. Una llegada tardía podía dejar al dispositivo sin
audio y el promedio comenzaba por debajo de su propia meta.

La reproducción arranca al reunir el margen previsto. Cargar un estado pausa,
limpia la cola y reinicia el promedio y la relación de frecuencia antes de volver
a llenarla. Se admite un error de una muestra por la división fraccional entre
frecuencia y fotogramas: evita esperar innecesariamente un quinto bloque.

Prueba SDL dummy en tiempo real: inicio con **4.224 muestras**, cola posterior
aprox. 3.528–4.754, relación 1,0000 y 59,73 fps. La base arrancaba por debajo de
2.112 muestras y corregía inicialmente la frecuencia a 0,9985. Guardar y cargar
un estado nuevo de la build también reinicia el llenado correctamente.

## Optimización sin cambiar el sonido

DutyIntegral calculaba sumas de los ocho segmentos de la onda dos veces por
muestra de salida. Se usan sumas prefijas y conversión entera de la fase positiva,
sin recorrerlos de nuevo ni llamar floor en el camino habitual. Una integral
independiente prueba **800.000 fases, error máximo 0** para los cuatro ciclos.

Una captura de música del mismo combate antes/después conserva correlación
0,9999999996 y nivel RMS 0,05618; la diferencia RMS es 0,000001586. No se ha
silenciado la música, desactivado la reverberación ni aplicado una ecualización
para esconder frecuencias. Esta comparación fue de síntesis, antes del resampler
del dispositivo; no mide por sí sola los cortes que provocaba una cola vacía.

## Mantener la corrección

```powershell
python port/tools/test_audio_render.py
```

La prueba compila con el toolchain del proyecto y no necesita ROM ni librerías
Python adicionales. Las funciones stub abortan si una ruta ajena al mezclador
se ejecuta accidentalmente. Mantener la independencia respecto a los bytes
posteriores al recurso; no volver a usar q[0] al final sin comprobar count.

Para una prueba de reproducción: KHCOM_AUDIOLOG=1, sin KHCOM_FAST, confirmar
«playback primed», cola estable y ausencia de descartes repetidos. Los dumps con
KHCOM_FAST sirven para comparar señales; su cola no representa reproducción a
velocidad real. No atribuir los tres fallos históricos de oclusión HUD del test
PPU a esta corrección de sonido.

Los quick states guardan punteros de la build: mantener el ejecutable v1.6.7
para la ranura antigua. Las partidas normales .sav sí se trasladan a v1.7.1.
