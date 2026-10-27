# Graph Execution

지원 node는 Input, Output, FormantShifter, CepstralMorph, TimbreResynth, InputBGenerator, Split, Merge다. graph는 DAG여야 하며 Input/Output은 각각 정확히 하나, required input은 정확히 하나의 edge를 가진다.

`CepstralMorph` SecondaryIn이 연결되면 external A/B overload를 사용한다. 없으면 P02 internal Input B를 사용한다. compiler는 두 입력의 cumulative latency 중 큰 값을 기준으로 짧은 edge에 `FixedDelay`를 삽입한다.

Merge도 동일한 방식으로 입력 latency를 보정하고 2-way 합에 1/sqrt(2)를 적용한다. Formant/Timbre latency는 2048, Cepstral latency는 selected FFT size다. output tail delay가 graph를 fixed 8192-sample host/report latency로 확장한다.
