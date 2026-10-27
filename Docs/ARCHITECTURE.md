# P01 아키텍처

`madron_dsp_core`는 JUCE와 독립적인 Float32 PCM view, lifecycle 계약, parameter metadata를 제공한다. `madron_formant`, `madron_cepstral`, `madron_timbre`는 각각 별도 static CMake target이다. `madron_random`은 DSP 인스턴스가 아닌 graph description과 validation만 담당한다.

JUCE `Madron` VST3는 세 engine을 prepare/reset하지만 P01에서는 host buffer를 변경하지 않는 stereo pass-through다. P02에서 각 engine의 core DSP를, P03에서 recipe generator와 graph transition을 추가한다.

VST3 identity는 manufacturer code `NyNa`, plugin code `Madr`, bundle ID `com.nynaudio.madron`으로 P01에서 고정했다. 이후 호환성을 위해 변경하지 않는다.
