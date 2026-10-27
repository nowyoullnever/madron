# Input B Generator

Input B is a standalone `madron_input_b` target. Filtered Copy is one-pole low-pass; Distorted Copy is normalized tanh saturation; Delayed Copy uses a preallocated non-feedback delay; Detuned Copy uses a moving cubic-interpolation read head in a preallocated ring. The last mode performs resampling rather than amplitude modulation. All modes are deterministic and silence-safe.
