# P02 validation report

## 수행한 정적/실행 검증

- Shared FFT/STFT: 사전 할당, silence/finite output, variable blocks를 P02 test executable에서 확인한다.
- Formant: 220 Hz + harmonic synthetic signal에서 700 cents 처리 후 latency를 제외한 output energy와 delayed dry와의 차이를 검사한다.
- Cepstral: external A/B와 Internal B path를 실제 frame 처리하고 finite output, energy 및 서로 다른 결과를 검사한다.
- Input B: filtered, distorted, delayed, detuned copy 네 모드에 대해 finite/non-silent output을 검사한다. Detuned mode는 moving fractional resampling read head를 사용한다.
- Timbre: 4096/1024 peak tracker가 sine input에서 event를 생성하는지, tracker→grain OLA가 wet output을 생성하는지, transpose/formant 설정 결과가 latency-aligned input과 달라지는지 검사한다.
- Graph: cycle 및 invalid audio bus rejection을 유지한다.

## 오프라인 결과

48 kHz, 1 second synthetic vowel, Debug renderer 결과:

| Engine | Parameters | RMS |
|---|---|---:|
| Formant | shift +700 cents | 0.935653 |
| Cepstral | Internal B default | 0.423324 |
| Timbre | transpose +1200 cents | 0.274714 |

결과 WAV는 build output에만 생성하며 Git에는 넣지 않는다. DAW host loading/listening test는 이 단계에서 수행하지 않았다.

## Latency 및 오프라인 시간

48 kHz 기준 reported latency는 Formant 2048 samples (42.67 ms), Cepstral 선택 FFT size N samples (N=1024이면 21.33 ms), Timbre 2048 samples (42.67 ms)다.

Windows Release `madron_render`, 2-second stereo synthetic vowel의 wall-clock 측정은 Formant 230.36 ms, Cepstral 989.84 ms, Timbre 78.17 ms였다. 이는 디스크 기록과 process startup을 포함한 단일 host 결과로, 실시간 CPU 사용률 또는 DAW 성능 보증이 아니다.
