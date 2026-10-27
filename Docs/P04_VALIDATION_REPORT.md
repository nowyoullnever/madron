# P04 Validation Report

## Build evidence

| Item | Result | Evidence |
| --- | --- | --- |
| Debug VST3 build | PASS | `cmake --build build/p03-vst-debug --config Debug --parallel 1` completed on 2026-10-27. |
| Release VST3 build | PASS | `cmake --build build/p03-vst-debug --config Release --parallel 1` completed on 2026-10-27. |
| Debug CTest | PASS | `madron_foundation`, 1/1, 7.52 s. |
| Release CTest | PASS | `madron_foundation`, 1/1, 0.53 s. |
| Release VST3 binary | PASS | `Madron.vst3/Contents/x86_64-win/Madron.vst3`; SHA-256 `088FF5ED7FF30F7F52D47949AC83EA2950DC03E04C6CABA72075712B4011B686`. |

## Static implementation checks

- New instances begin with `mix = 0.5` and `recipeActive = false`; until activation the input is forwarded through the latency-matched dry delay.
- `mix` is a JUCE `AudioParameterFloat`; UI updates call `setValueNotifyingHost`, and the audio callback applies a per-sample smoother shared by L/R.
- Recipe compilation occurs in the worker `std::jthread`; no recipe compiler or graph allocation path is called by `processBlock`.
- Graph retirement collection runs from the worker even if the editor is closed.
- The editor paints only the product wordmark, Lucky label, unlabelled Mix control, and moon/sun glyph. Gowun Batang Regular is embedded as JUCE BinaryData.

## Manual GUI and host validation

FL Studio 2025 was detected running on the validation workstation. Its active `Project_1.flp` contained an existing user session and plug-in chain. No existing Madron installation was found in the shared VST3 directory, but the attempted non-overwriting copy to `C:\Program Files\Common Files\VST3\Madron.vst3` was denied by filesystem permissions. The VST3 was therefore not inserted into that project. Consequently, the following remain **NOT RUN**: FL Studio rescan, insertion, parameter automation, preset restore, audio playback, CPU profiling, and visual screenshot capture. They must not be inferred from the build or CTest results.

## Remaining release risk

The binary/container and core test target are verified. A clean disposable host session remains required before declaring DAW-host compatibility or audible behaviour verified.
