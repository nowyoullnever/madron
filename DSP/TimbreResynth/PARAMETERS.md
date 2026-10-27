# Parameters

- `timbre.transpose_cents`: synthesis mark rate를 변경한다.
- `timbre.formant_cents`: grain source read stride를 변경한다.
- `timbre.target_hz`: 0이면 tracked frequency×transpose, 양수이면 target scheduler frequency.
- `timbre.active_voices`: 1..20 fixed voice pool limit.
- `timbre.wet_dry`: 2048-sample delayed dry와 grain wet을 결합한다.
