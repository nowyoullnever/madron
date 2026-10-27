# Formant Shifter implementation

REA 직접 근거: 2048/4 pfft, regional mean보다 큰 local peak 선택, peak/end-point linear interpolation, `s=2^(shift/12)`, `M/E × E[k/s]`, 원 phase 보존, `k<6` 및 high edge mask다. P02는 cents를 `2^(cents/1200)`으로 변환하고 동일 순서의 spectral envelope shift를 수행한다.

독립 구현: sqrt-Hann window/OLA normalization, epsilon divide guard, P01 확장 range(-2400..2400 cents, width 1..128), atomic parameter/smoothing이다. 원본 window scale과 divide-zero 동작은 REA에서 UNKNOWN이므로 동등성을 주장하지 않는다.
