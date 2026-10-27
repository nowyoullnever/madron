# DSP 인터페이스

`AudioBufferView`와 `MutableAudioBufferView`는 메모리를 소유하지 않는다. caller가 Float32 planar PCM을 보유하며, DSP는 `prepare(ProcessSpec)`에서만 buffer/plan을 확보해야 한다. `process`에는 allocation, I/O, mutex, FFT-plan 생성이 허용되지 않는다.

P01 core는 44.1/48/96 kHz 및 mono/stereo view를 허용한다. plugin host 정책은 stereo-in/stereo-out만 허용한다. `copyAudio`는 동일 channel의 in-place alias를 지원하지만 cross-channel overlap은 지원하지 않는다. Cepstral은 두 번째 input view를 받지만 P01 output은 A pass-through다.

모든 future parameter는 stable ID, range, default, unit, mapping, smoothing, randomizable, lockable metadata를 가져야 한다.

세 engine header는 stable descriptor 배열을 노출한다. `normalizedToValue`는 Linear, 양수 범위 Exponential, Enumerated 반올림을 실제로 분기한다. 여기의 range/default는 Madron의 독립 구현 계약이며, 원본 REA에서 직접 확정된 UI 범위라고 주장하지 않는다.

`ParameterDescriptor`와 `AtomicParameter`는 제어 스레드에서 변경한 scalar 값을 오디오 스레드에 전달하기 위한 P01 공통 계약이다. P02 engine 구현은 처리 시작 시 atomic snapshot을 읽고, descriptor의 smoothing 정책을 적용한다. P01 pass-through engine은 파라미터를 아직 오디오 계산에 사용하지 않는다.

`TimbreResynth::enqueuePartial`은 tracker producer 하나와 audio consumer 하나를 위한 고정 길이 SPSC event queue를 사용한다. push/pop은 allocation이나 mutex 없이 수행하며, lifecycle의 `prepare`/`reset`은 producer와 audio callback이 동시 실행하지 않는 host 상태에서만 호출한다. P01에서는 event를 고정 `activePartials` 슬롯으로 전달만 하고 합성하지 않는다.
