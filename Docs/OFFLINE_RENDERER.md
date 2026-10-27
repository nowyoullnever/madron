# Offline renderer

`madron_render`는 DSP library를 직접 링크하는 독립 CLI다. PCM16 및 Float32 WAV mono/stereo 입력을 읽고 Float32 WAV를 기록한다. 입력을 생략하면 재현 가능한 `sine`, `vowel`, `noise`, `impulse` 신호를 생성한다.

```powershell
madron_render --engine formant --generate vowel --seconds 1 --shift-cents 700 --output formant.wav
madron_render --engine cepstral --input input.wav --output cepstral.wav
madron_render --engine timbre --generate vowel --seconds 1 --shift-cents 1200 --output timbre.wav
```

Renderer는 plugin과 동일한 `madron_formant`, `madron_cepstral`, `madron_timbre` targets를 사용한다. 원본 AMXD, Max external, 원본 음악 파일은 사용하거나 포함하지 않는다.
