# Presets and State

Recipe JSON은 schema/generator version, seed, category, intensity, complexity, performance, graph nodes, connections, per-node parameter, node lock, structure lock, parameter lock을 저장한다. parser는 schema 1만 허용하고 잘못된 숫자·category·graph를 거부한다.

`RecipeHistory`는 기본 32 entries의 undo/redo cursor와 favorite snapshot을 보유한다. undo 뒤 새로운 push는 redo tail을 제거한다. plugin Save/Load는 JSON 파일을 사용하고 DAW state에는 현재 JSON과 global wet을 기록한다.

P03 기능 검증용 GUI는 Structure Lock, Node ID 기반 node lock, Parameter ID 기반 parameter lock을 제공한다. graph summary에서 node 순서를 확인한 뒤 ID/parameter ID를 입력해 잠글 수 있다. 더 나은 node inspector와 lock 상태 시각화는 P04 GUI 작업이다.
