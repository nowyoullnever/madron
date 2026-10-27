# Madron 1.0.0 Release Notes

## Included

- Windows x64 VST3 audio effect: `Madron.vst3`
- Minimalist white/black editor with embedded Gowun Batang Regular
- Initial inactive safe path, global host-visible Mix parameter (default 0.5), and deterministic Lucky recipe activation/reroll
- Background graph compilation and graph-retirement collection outside the audio callback
- Existing Formant Shifter, Cepstral Morph, Timbre Resynth, Input B Generator, and Random Recipe graph engines

## Installation

1. Close the target DAW.
2. Copy the supplied `Madron.vst3` folder to the DAW's VST3 plug-in directory.
3. Rescan plug-ins in the DAW and insert Madron as a stereo audio effect.

The archive does not install or modify any host configuration automatically.

## Known boundaries

- The bundled Timbre Resynth follows an independent grain/OLA policy; it is not asserted to be sample-equivalent to the original AMXD `psych~` external.
- This release is Windows x64 VST3 only.
- Host-specific manual checks are recorded only when actually completed in `P04_VALIDATION_REPORT.md`.
