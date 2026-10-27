# Parameters

- `formant.shift_cents`: -2400..2400 cents. 내부 ratio `2^(cents/1200)`.
- `formant.envelope_width`: 1..128 FFT bins. 원본 UI의 확인 범위(0..16)와 Madron 확장 범위를 구분한다.
- `formant.wet_dry`: 0..1. dry path는 2048 samples delay compensated다.
