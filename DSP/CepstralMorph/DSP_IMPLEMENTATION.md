# Cepstral Morph implementation

REA 직접 근거: four FFT variants N=512/1024/2048/4096, overlap 8, log magnitude, cepstrum, `<25`/`>25` mask, envelope/detail 각각의 linear mix, Normal/Inverse/A/B four phase outputs다. P02는 low quefrency `q<=25 || q>=N-25`를 envelope로, 나머지를 detail로 사용해 `exp(lerp(E)+lerp(D))`를 재구성한다.

독립 구현: phase는 raw linear blend 뒤 [-pi,pi]으로 wrap하며, Internal B는 Madron의 Filtered/Distorted/Delayed/Detuned generator를 사용한다. 원본 XY/Intensity mapping, raw phase wrap, log-zero 정책은 UNKNOWN이다.
