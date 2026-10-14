#!/usr/bin/env python3
"""Compare the port's audio with an emulator recording of the same span.

1. Reference: BizHawk from power-on, no input, as a WAV:
       EmuHawk.exe --dump-type=wave --dump-name=ref.wav --dump-length=4200 --dump-close roms/B8CE.gba
2. Port, same span:
       KHCOM_FAST=1 KHCOM_QUIT=4200 KHCOM_WAVDUMP=port.f32 khcom.exe
   (raw float32 stereo; the rate of each block is in port.f32.rates)
3. Compare (needs numpy and scipy, e.g. `uv run --with numpy --with scipy`):
       python port/tools/audio_compare.py ref.wav port.f32 [skip_seconds]
   (skip_seconds drops the start of both, e.g. menus that the emulator's lag
   frames make last longer, before aligning)

Aligns the two by their envelopes, then reports per 5 s window the level
difference, envelope and sample correlation, the timing drift and the
energy difference per octave band.
"""
import sys

import numpy as np
from scipy.io import wavfile
from scipy.signal import correlate, resample_poly
ref_path, port_path = sys.argv[1], sys.argv[2]
sr, ref = wavfile.read(ref_path); ref = ref.astype(np.float64) / 32768.0
blocks = [tuple(map(int, l.split())) for l in open(port_path + ".rates")]
raw = np.fromfile(port_path, dtype=np.float32).reshape(-1, 2).astype(np.float64)
parts, pos, i = [], 0, 0
while i < len(blocks):  # resample each run of blocks at one rate
    rate, n = blocks[i][1], 0
    while i < len(blocks) and blocks[i][1] == rate:
        n += blocks[i][0]
        i += 1
    g = np.gcd(sr, rate)
    parts.append(resample_poly(raw[pos:pos + n], sr // g, rate // g, axis=0))
    pos += n
port = np.concatenate(parts)
if len(sys.argv) > 3:
    skip = int(float(sys.argv[3]) * sr)
    ref, port = ref[skip:], port[skip:]
print(f"ref {len(ref)/sr:.1f}s @{sr} ch{ref.shape[1]}  port {len(port)/sr:.1f}s")
def env(x, hop=441):
    m = x.mean(axis=1); n = len(m)//hop
    return np.sqrt((m[:n*hop].reshape(n, hop)**2).mean(axis=1))
er, ep = env(ref), env(port)
c = correlate(er - er.mean(), ep - ep.mean(), mode="full"); lag = np.argmax(c) - (len(ep) - 1)
print(f"alignment: port starts {lag*10} ms later in the reference")
off = lag * 441
a = ref[max(off,0):]; b = port[max(-off,0):]; n = min(len(a), len(b)); a, b = a[:n], b[:n]
W = sr * 5
bands = [(20,45),(45,90),(90,180),(180,360),(360,720),(720,1440),(1440,2880),(2880,5760),(5760,11520)]
print("win  t(s)  rmsRef  rmsPort  dB   envCorr  bestLag(ms) sampCorr  " + " ".join(f"{lo}-{hi}Hz" for lo,hi in bands))
tot=[]
for i in range(n // W):
    x, y = a[i*W:(i+1)*W].mean(axis=1), b[i*W:(i+1)*W].mean(axis=1)
    rx, ry = np.sqrt((x**2).mean()), np.sqrt((y**2).mean())
    if rx < 1e-3 and ry < 1e-3: continue
    ex, ey = env(a[i*W:(i+1)*W]), env(b[i*W:(i+1)*W])
    ec = np.corrcoef(ex, ey)[0,1]
    cc = correlate(x, y, mode="full", method="fft"); mid = len(y)-1; s = 441*3
    k = np.argmax(cc[mid-s:mid+s]) - s; sc = cc[mid+k] / (np.sqrt((x**2).sum()*(y**2).sum()) + 1e-12)
    X, Y = np.abs(np.fft.rfft(x))**2, np.abs(np.fft.rfft(y))**2; f = np.fft.rfftfreq(len(x), 1/sr)
    bd = [10*np.log10((Y[(f>=lo)&(f<hi)].sum()+1e-12)/(X[(f>=lo)&(f<hi)].sum()+1e-12)) for lo,hi in bands]
    tot.append((20*np.log10(ry/rx+1e-12), ec, sc, bd))
    print(f"{i:3d} {i*5:5d} {rx:7.4f} {ry:7.4f} {20*np.log10(ry/rx+1e-12):5.1f} {ec:7.3f} {k*1000/sr:9.2f} {sc:8.3f}  " + " ".join(f"{v:+6.1f}" for v in bd))
t = np.array([[d, e, s] for d, e, s, _ in tot]); B = np.array([bd for *_, bd in tot])
print(f"mean: level {t[:,0].mean():+.2f} dB, envelope corr {t[:,1].mean():.3f}, sample corr {t[:,2].mean():.3f}, bands " + " ".join(f"{v:+.1f}" for v in B.mean(axis=0)))
