#pragma once

#include <madron/dsp/Parameter.h>
#include <madron/dsp/Processor.h>

#include <array>
#include <atomic>
#include <cstddef>

namespace madron::timbre {
struct PartialEvent { float frequencyHz {}; float amplitude {}; std::size_t voiceIndex {}; };
struct Parameters { float transposeCents {}; float formantCents {}; float wetDry { 1.0f }; std::size_t activeVoices { 20 }; };
class TimbreResynth final : public dsp::AudioProcessor {
public:
    static constexpr std::size_t maximumVoices = 20;
    static constexpr std::size_t eventQueueCapacity = 64;
    static constexpr std::array parameterDescriptors {
        dsp::ParameterDescriptor { "timbre.transpose_cents", "Transpose", dsp::ParameterUnit::Cents, -2400.0f, 2400.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "timbre.formant_cents", "Formant", dsp::ParameterUnit::Cents, -2400.0f, 2400.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "timbre.wet_dry", "Wet / Dry", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 1.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "timbre.active_voices", "Active Voices", dsp::ParameterUnit::Choice, 1.0f, 20.0f, 20.0f, dsp::ParameterMapping::Enumerated, dsp::SmoothingPolicy::None, true, true }
    };
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept override;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override { return 0; }
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }
    void setParameters(const Parameters& parameters) noexcept;
    bool enqueuePartial(const PartialEvent& event) noexcept;
private:
    bool dequeuePartial(PartialEvent& event) noexcept;
    dsp::ProcessSpec spec_ {}; Parameters parameters_ {}; bool prepared_ {};
    std::array<PartialEvent, eventQueueCapacity> eventQueue_ {};
    std::array<PartialEvent, maximumVoices> activePartials_ {};
    std::atomic<std::size_t> eventWrite_ { 0 };
    std::atomic<std::size_t> eventRead_ { 0 };
};
} // namespace madron::timbre
