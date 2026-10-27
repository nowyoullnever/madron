# Madron

Madron은 Formant Shifter, Cepstral Morph, Timbre Resynth를 독립 DSP engine으로 구성하고, seed 기반 Random Recipe graph로 직렬·병렬·분기·재결합하는 랜덤 스펙트럴 멀티이펙트 VST3다.

## 1.0.0 구현 범위

- 순수 C++20 `madron_dsp_core` 및 독립 `madron_formant`, `madron_cepstral`, `madron_timbre` targets
- prepare/reset/process/latency 계약과 Float32 planar buffer view
- SharedDSP radix-2 FFT/STFT/OLA/window-normalization/dry delay
- 실제 Formant spectral-envelope shift, Cepstral A/B morph, waveform-grain Timbre resynthesis
- Internal Input B Generator: filtered/distorted/delayed/detuned copy
- DAG graph model, resource limit/port/cycle/audio-bus validator
- JUCE 8.0.4 고정 VST3 Audio Effect (`Madron`), Stereo I/O, diagnostic engine selector
- WAV offline renderer와 Release에서도 실행되는 명시적 require 기반 CTest

P03의 Dynamic Graph Runtime, deterministic Random Recipe, graph-level preset/history/state, equal-power graph transition, offline random renderer를 유지한다. 1.0.0은 이를 최소 UI로 전환했다. 플러그인 창에는 `madron`, `I'm Feeling Lucky`, 무표기 Mix, 달/해 테마 토글만 표시한다. 새로운 인스턴스는 비활성 안전 경로와 Mix 0.5로 시작하며, Lucky 클릭 후 비동기 graph compile이 완료되면 효과가 활성화된다.

`psych~` 분석에서 확인된 waveform/grain overlap-add 방향성을 TimbreResynth의 input-ring, 8x auxiliary stream, cubic interpolation, windowed OLA 구조에 반영했다. 정확한 `psych~` window/gain/tracker 구현은 확보되지 않았으므로 독립 정책이며 원본 동등성을 주장하지 않는다.

## 빌드

`Docs/BUILD.md`의 CMake preset 절차를 사용한다. Release VST3 expected output은 `build/windows-release/Plugin/Madron_artefacts/Release/VST3/Madron.vst3`이며 자동 설치는 하지 않는다. 릴리스 ZIP에서는 `Madron.vst3`를 호스트의 VST3 경로에 수동으로 복사한다. Offline renderer 사용법은 `Docs/OFFLINE_RENDERER.md`에 있다.

## 1.0.0 검증 범위

`Docs/P04_VALIDATION_REPORT.md`에 Debug/Release 빌드와 CTest, VST3 컨테이너 검증, GUI/호스트 검증의 실제 수행 범위를 기록한다. 원본 `psych~` 구현은 확보되지 않았으므로 Timbre Resynth는 독립적인 설계이며 원본과의 동등성을 주장하지 않는다.
