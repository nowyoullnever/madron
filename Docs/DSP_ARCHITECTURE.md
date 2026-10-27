# P02 DSP 아키텍처

`madron_dsp_core`의 `FFTPlan`과 `StreamingStft`는 512/1024/2048/4096 radix-2 FFT, sqrt-Hann analysis/synthesis window, per-channel input ring, OLA와 sample-wise normalization을 제공한다. frame 처리와 모든 scratch memory는 `prepare()`에서 확보한다. `process()`는 allocation, I/O, mutex 및 FFT plan 생성을 수행하지 않는다.

Formant Shifter는 2048-point/overlap 4이며 실제 처리 지연은 2048 samples다. Cepstral Morph는 4개 N/8 state를 모두 사전 준비하므로 audio callback에서 FFT size 변경을 위한 allocation이 없다. 선택 상태의 latency는 N samples다. Timbre Resynth는 tracker 4096/1024와 grain output 2048-sample look-ahead를 사용하며, 현재 전체 처리 지연은 2048 samples다.

공통 `FixedDelay`는 dry/wet을 STFT/OLA 결과와 같은 시간 축에서 결합한다. parameter setter는 atomics로 clamp된 scalar를 저장하고, frame/sample state는 one-pole/linear smoothing으로 읽는다.
