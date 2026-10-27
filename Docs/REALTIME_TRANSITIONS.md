# Realtime Transitions

RANDOM은 audio callback에서 graph를 생성하지 않는다. UI thread가 recipe generate/validate/compile/prepare를 완료한 뒤 pending graph를 publish한다. callback은 block 경계에서 이를 채택하고 old/new를 50–200 ms equal-power crossfade 한다.

모든 plugin graph는 fixed 8192 latency이므로 transition 양쪽 출력은 이미 latency-aligned다. 빠른 roll은 pending graph의 Latest-Wins 정책을 쓴다. old/incoming graph의 파괴는 callback 밖 `collectRetired()`가 수행한다.

제한: host 내부 DAW 자동화와 실제 host latency renegotiation은 P04 host QA 대상으로 남는다. P03은 plugin이 `setLatencySamples(8192)`로 고정 latency를 보고한다.
