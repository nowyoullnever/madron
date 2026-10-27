# Madron

Madron은 Formant Shifter, Cepstral Morph, Timbre Resynth를 독립 DSP engine으로 구성하고, seed 기반 Random Recipe graph로 직렬·병렬·분기·재결합하는 랜덤 스펙트럴 멀티이펙트 VST3다.

## P02 구현 범위

- 순수 C++20 `madron_dsp_core` 및 독립 `madron_formant`, `madron_cepstral`, `madron_timbre` targets
- prepare/reset/process/latency 계약과 Float32 planar buffer view
- SharedDSP radix-2 FFT/STFT/OLA/window-normalization/dry delay
- 실제 Formant spectral-envelope shift, Cepstral A/B morph, waveform-grain Timbre resynthesis
- Internal Input B Generator: filtered/distorted/delayed/detuned copy
- DAG graph model, resource limit/port/cycle/audio-bus validator
- JUCE 8.0.4 고정 VST3 Audio Effect (`Madron`), Stereo I/O, diagnostic engine selector
- WAV offline renderer와 Release에서도 실행되는 명시적 require 기반 CTest

P03은 Dynamic Graph Runtime, deterministic Random Recipe, graph-level preset/history/state, equal-power graph transition, offline random renderer와 기능 검증용 VST3 controls를 구현했다. 상세 lock editor·host QA·최종 GUI는 P04 범위다.

`psych~` 분석에서 확인된 waveform/grain overlap-add 방향성을 TimbreResynth의 input-ring, 8x auxiliary stream, cubic interpolation, windowed OLA 구조에 반영했다. 정확한 `psych~` window/gain/tracker 구현은 확보되지 않았으므로 독립 정책이며 원본 동등성을 주장하지 않는다.

## 빌드

`Docs/BUILD.md`의 CMake preset 절차를 사용한다. Release VST3 expected output은 `build/windows-release/Plugin/Madron_artefacts/Release/VST3/Madron.vst3`이며 자동 설치는 하지 않는다. Offline renderer 사용법은 `Docs/OFFLINE_RENDERER.md`에 있다.
