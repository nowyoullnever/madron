#pragma once

#include <madron/dsp/Parameter.h>
#include <madron/dsp/Processor.h>
#include <madron/dsp/Spectral.h>

#include <array>
#include <atomic>
#include <vector>

namespace madron::formant {
struct Parameters { float shiftCents { 0.0f }; float envelopeWidth { 10.0f }; float wetDry { 1.0f }; };

class FormantShifter final : public dsp::AudioProcessor {
public:
    static constexpr std::size_t fftSize = 2048, overlap = 4;
    static constexpr std::array parameterDescriptors {
        dsp::ParameterDescriptor { "formant.shift_cents", "Formant Shift", dsp::ParameterUnit::Cents, -2400.0f, 2400.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "formant.envelope_width", "Envelope Width", dsp::ParameterUnit::Generic, 1.0f, 128.0f, 10.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "formant.wet_dry", "Wet / Dry", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 1.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true }
    };
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept override;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override { return fftSize; }
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }
    void setParameters(const Parameters& parameters) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept;
private:
    struct Channel { dsp::StreamingStft stft {}; dsp::FixedDelay dryDelay {}; std::vector<float> magnitude {}, envelope {}; float smoothShiftCents {}, smoothWet { 1.0f }; };
    struct CallbackContext { FormantShifter* owner {}; Channel* channel {}; };
    static void processSpectrum(void* context, std::span<dsp::Complex> spectrum) noexcept;
    void shapeSpectrum(Channel& channel, std::span<dsp::Complex> spectrum) noexcept;
    dsp::ProcessSpec spec_ {}; std::vector<Channel> channels_ {};
    std::atomic<float> shiftCents_ { 0.0f }, envelopeWidth_ { 10.0f }, wetDry_ { 1.0f };
    bool prepared_ {};
};
} // namespace madron::formant
