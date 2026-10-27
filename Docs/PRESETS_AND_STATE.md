# Presets and State

Recipe JSON은 schema/generator version, seed, category, intensity, complexity, performance, graph nodes, connections, per-node parameter, node lock, structure lock, parameter lock을 저장한다. parser는 schema 1만 허용하고 잘못된 숫자·category·graph를 거부한다.

`RecipeHistory`는 기본 32 entries의 undo/redo cursor와 favorite snapshot을 보유한다. undo 뒤 새로운 push는 redo tail을 제거한다. plugin Save/Load는 JSON 파일을 사용하고 DAW state에는 현재 JSON과 global wet을 기록한다.

현재 P03 GUI는 graph-level structure lock/개별 parameter lock 편집기를 노출하지 않는다. lock data model, generator 보존 규칙, preset/state round-trip API는 구현되어 있다. 세부 lock editor는 P04 GUI 작업이다.
