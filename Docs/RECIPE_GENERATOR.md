# P03 Recipe Generator

PRNG는 구현 정의 분포를 쓰지 않는 xorshift64*이며 generator/schema version은 1이다. 동일 seed/category/intensity/complexity/performance/lock은 같은 graph와 parameter를 만든다.

10 category: `formant-mutation`, `spectral-fusion`, `harmonic-destruction`, `pitch-deconstruction`, `granular-reconstruction`, `texture-mutation`, `spectral-smearing`, `unstable-transformation`, `extreme-processing`, `subtle-transformation`.

Category는 engine weight, cross-source/parallel 확률, wet, envelope/detail, timbre voices 및 parameter 범위를 바꾼다. Intensity는 parameter 폭과 wet, Complexity는 serial length와 parallel 선택 확률을 제어한다. Eco/Balanced/Extreme은 각각 node/cost budget 8/350, 14/650, 20/950으로 제한한다.

생성 topology는 single/serial, split-parallel-merge, explicit-B cross-source Cepstral을 포함한다. `InputBGenerator`는 cross-source B 또는 standalone node가 된다. validator는 엔진 타입·FFT·voices에서 비용을 재계산하며 caller의 `ResourceCost`를 신뢰하지 않는다.
