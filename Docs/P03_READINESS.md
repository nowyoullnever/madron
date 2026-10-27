# P03 readiness

각 DSP instance는 independent buffer/state를 가진다. P03 adapter는 `NodeInstance`마다 FormantShifter, CepstralMorph 또는 TimbreResynth를 하나씩 생성하고, `GraphConnection`의 branch buffer를 input view로 제공하면 된다. Cepstral은 external A/B overload 및 Internal B overload를 모두 가진다. output latency는 engine 별 `getLatencySamples()`로 받아 merge 전 delay compensation을 배치해야 한다.

현재 random graph generator, branch buffer pool, engine crossfade는 구현하지 않았다. Graph 교체 시에는 새 instance를 non-audio thread에서 `prepare()`하고, old/new outputs를 latency-aligned crossfade하는 정책을 사용한다.
