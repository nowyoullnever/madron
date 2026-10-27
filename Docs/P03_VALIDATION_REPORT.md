# P03 Validation Report

## Executed

- Debug CTest: `madron_foundation` 1/1 passed, 6.31 s. P02 DSP checks와 P03 1000 deterministic recipe validation, seed equality, topology diversity, JSON round-trip, explicit-B graph audio processing, history, structure lock, transition finite-output checks를 포함한다.
- Release CTest: 1/1 passed, 0.44 s.
- Debug/Release VST3 builds completed from `build/p03-vst-debug`.
- Release renderer: 48 kHz stereo synthetic vowel, 2 s, seed 123456, intensity 75, complexity 60, Balanced. wall clock은 Formant Mutation 157.62 ms, Spectral Fusion 1009.74 ms, Granular Reconstruction 86.65 ms였다. process startup/file I/O를 포함하므로 DAW realtime CPU 수치가 아니다.

## Output

`random-p03-release.wav`와 `random-p03-release.json`은 Release renderer가 실제 `GraphCompiler`/`ExecutableGraph` path로 만들었다. graph는 `Input -> Formant -> InputB -> Cepstral -> Output`, renderer graph latency는 3072 samples다. plugin fixed latency는 8192 samples다.

## Limits

- DAW/FL Studio load, audible comparison, long-duration host stress test는 수행하지 않았다.
- P03 GUI는 ID 기반 Structure/Node/Parameter lock을 제공하지만, lock 상태를 그래프 위에 시각화하는 inspector는 P04 대상이다.
- CPU estimator는 engine configuration based budget이며 user hardware profiler는 아니다.
