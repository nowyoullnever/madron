# Madron

Madron은 Formant Shifter, Cepstral Morph, Timbre Resynth를 독립 DSP engine으로 구성하고, 향후 seed 기반 Random Recipe graph로 직렬·병렬·분기·재결합할 랜덤 스펙트럴 멀티이펙트 VST3다.

## P01 구현 범위

- 순수 C++20 `madron_dsp_core` 및 독립 `madron_formant`, `madron_cepstral`, `madron_timbre` targets
- prepare/reset/process/latency 계약과 Float32 planar buffer view
- P01의 안전한 pass-through 처리 및 unit test
- DAG graph model, resource limit/port/cycle validator
- JUCE 8.0.4 고정 VST3 Audio Effect (`Madron`), Stereo I/O, state persistence, Foundation Build GUI

실제 Formant STFT, Cepstral synthesis, partial tracker, PSOLA grain synthesis, recipe generation은 P02/P03 범위다. P01은 이 기능들을 구현됐다고 주장하지 않는다.

`psych~` 분석에서 확인된 waveform/grain overlap-add 계열 구조는 TimbreResynth의 후속 설계 근거일 뿐, P01은 이를 일반 sine/additive 합성으로 대체하지 않는다. 현재 Timbre target은 partial event를 고정 크기 queue로 받아 미래 voice slot에 전달하고, 오디오는 명시적으로 pass-through한다.

## 빌드

`Docs/BUILD.md`의 CMake preset 절차를 사용한다. Release VST3 expected output은 `build/windows-release/Plugin/Madron_artefacts/Release/VST3/Madron.vst3`이며 자동 설치는 하지 않는다.
