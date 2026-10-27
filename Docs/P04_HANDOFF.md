# P04 Handoff

- 1.0.0은 최소 UI와 Release ZIP을 제공한다. graph inspector와 individual lock editor는 의도적으로 노출하지 않는다.
- `graphSummary()`는 더 이상 graph를 컴파일하지 않는다. GUI에 상태 문자열도 표시하지 않는다.
- 다음 릴리스 전 SIMD/profile 기반 budget calibration, Cepstral parallel-variant cost reduction, memory telemetry를 수행한다.
- FL Studio 및 다른 VST3 host에서 fixed 8192 latency, state restore, rapid roll, 자동화, Save/Load, 장시간 청취를 검증한다. P04 시점에 이 host 검증은 실행되지 않았으며 상세는 `P04_VALIDATION_REPORT.md`에 있다.
- 원본 AMXD `psych~` DSP의 동등성은 아직 입증되지 않았다.
