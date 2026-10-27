# Timbre Resynth implementation

REA 직접 근거: `psych~`는 input/full/decimated/lag/OLA buffers, 8-sample average stream, candidate finder, fractional four-point interpolation, precomputed window 및 overlap-add grain output을 가진다. `sigmund~` patch 설정은 4096 npts, 1024 hop, npeak 20, maxfreq 8000 Hz다.

P02 독립 구현: `PartialTracker`는 windowed FFT local-peak/parabolic interpolation, nearest-frequency track matching과 start/update/stop event를 사용한다. `GrainResynthesizer`는 input ring에서 cubic interpolation으로 waveform grain을 읽고 Hann window OLA로 미래 buffer에 합성한다. transpose는 synthesis-mark rate, formant는 grain read stride에 실제 적용된다. 20 voice pool, amplitude smoothing 및 voice stealing을 제공한다.

원본의 candidate equation 전체, psych window identity/gain, exact reset/latency, sigmund clock-thread track policy는 UNKNOWN이다. 이 구현은 원본 바이너리 재사용이나 동일성 주장이 아닌 독립 DSP다.
