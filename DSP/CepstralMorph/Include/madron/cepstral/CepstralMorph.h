#pragma once

#include <madron/dsp/Parameter.h>
#include <madron/dsp/Processor.h>

#include <array>

namespace madron::cepstral {
enum class PhaseMode { Normal, Inverted, AOnly, BOnly };
enum class FFTSize : std::size_t { N512 = 512, N1024 = 1024, N2048 = 2048, N4096 = 4096 };
struct Parameters { FFTSize fftSize { FFTSize::N1024 }; float envelopeBlend {}; float detailBlend {}; float phaseBlend {}; PhaseMode phaseMode { PhaseMode::Normal }; };
class CepstralMorph final : public dsp::AudioProcessor {
public:
    static constexpr std::array parameterDescriptors {
        dsp::ParameterDescriptor { "cepstral.fft_size", "FFT Size", dsp::ParameterUnit::Choice, 0.0f, 3.0f, 1.0f, dsp::ParameterMapping::Enumerated, dsp::SmoothingPolicy::None, false, true },
        dsp::ParameterDescriptor { "cepstral.envelope_blend", "Envelope Blend", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "cepstral.detail_blend", "Detail Blend", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "cepstral.phase_blend", "Phase Blend", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "cepstral.phase_mode", "Phase Mode", dsp::ParameterUnit::Choice, 0.0f, 3.0f, 0.0f, dsp::ParameterMapping::Enumerated, dsp::SmoothingPolicy::None, true, true }
    };
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& inputA, const dsp::MutableAudioBufferView& output) noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& inputA, const dsp::AudioBufferView& inputB, const dsp::MutableAudioBufferView& output) noexcept;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override { return 0; }
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }
    void setParameters(const Parameters& parameters) noexcept { parameters_ = parameters; }
    [[nodiscard]] Parameters parameters() const noexcept { return parameters_; }
private:
    dsp::ProcessSpec spec_ {}; Parameters parameters_ {}; bool prepared_ {};
};
} // namespace madron::cepstral
