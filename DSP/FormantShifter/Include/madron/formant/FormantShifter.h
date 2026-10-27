#pragma once

#include <madron/dsp/Parameter.h>
#include <madron/dsp/Processor.h>

#include <array>

namespace madron::formant {

struct Parameters {
    float shiftCents { 0.0f };
    float envelopeWidth { 10.0f };
    float wetDry { 1.0f };
};

class FormantShifter final : public dsp::AudioProcessor {
public:
    static constexpr std::size_t fftSize = 2048;
    static constexpr std::size_t overlap = 4;
    static constexpr std::array parameterDescriptors {
        dsp::ParameterDescriptor { "formant.shift_cents", "Formant Shift", dsp::ParameterUnit::Cents, -2400.0f, 2400.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "formant.envelope_width", "Envelope Width", dsp::ParameterUnit::Generic, 1.0f, 128.0f, 10.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "formant.wet_dry", "Wet / Dry", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 1.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true }
    };
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept override;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override { return 0; } // P02 replaces with measured STFT latency.
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }
    void setParameters(const Parameters& parameters) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept { return parameters_; }
private:
    dsp::ProcessSpec spec_ {};
    Parameters parameters_ {};
    bool prepared_ {};
};

} // namespace madron::formant
