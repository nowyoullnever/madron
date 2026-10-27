# Random Graph 명세

`NodeType`은 engine 종류, `NodeInstance`는 실제 상태와 parameter를 가진 개별 graph node다. 따라서 Formant node 두 개는 별도 NodeID·ParameterSet을 가진다. `GraphConnection`은 output에서 named input port로의 단방향 audio edge와 branch gain을 표현한다.

`GraphConnection`은 branch gain과 source/destination channel count를 함께 보유한다. P01 validator는 1/2 channel 외 구성과 channel-count 불일치를 거부한다. 또한 중복 NodeID, 존재하지 않는 node, 부적합 port, input fan-in, 필수 input 누락, node/cost limit, directed cycle을 거부한다. Cepstral의 secondary input은 선택적이고, Merge는 두 필수 input을 가진다. feedback loop는 P01에서 의도적으로 불가하다.

`RecipeDefinition`, seed, locks, constraints가 이미 모델에 있으므로 P03는 weighted selection, history, serialization, CPU-aware generation과 crossfade state를 추가할 수 있다.
