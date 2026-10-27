# P03 Random Engine Architecture

`RecipeGenerator`는 고정 프리셋 배열을 선택하지 않는다. 고정된 xorshift64* PRNG와 seed를 사용해 `GraphDescription`의 노드, 연결, 파라미터를 생성한다. `GraphCompiler`는 validate → topological sort → 실제 DSP factory → latency/buffer plan → prepared `ExecutableGraph` 순으로 non-audio thread에서 수행한다.

`ExecutableGraph`는 node output buffer slot을 마지막 소비 시점 뒤에 재사용한다. edge별 scratch/delay buffer와 DSP instance도 compile 때만 할당된다. `process()`는 이미 준비된 buffer, delay와 DSP만 호출한다.

`GraphTransition`은 atomic pending pointer의 Latest-Wins 정책을 사용한다. 새 graph는 UI/non-audio thread에서 compile한 뒤 request되고, audio thread에서는 old/new graph를 equal-power crossfade 한다. retirement SPSC queue의 delete는 UI timer가 `collectRetired()`로 수행한다.

플러그인은 graph 출력 latency를 8192 samples로 고정한다. 각 graph의 output tail delay와 global dry delay를 같은 값으로 맞추므로 graph 변경 전후 및 global wet/dry가 같은 시간축을 사용한다. 8192 samples를 넘는 graph는 compiler가 거부한다.
