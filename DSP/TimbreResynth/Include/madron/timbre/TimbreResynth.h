#pragma once

#include <madron/dsp/Parameter.h>
#include <madron/dsp/Processor.h>
#include <madron/dsp/Spectral.h>
#include <madron/timbre/GrainResynthesizer.h>
#include <madron/timbre/PartialTracker.h>

#include <array>
#include <atomic>
#include <cstddef>
#include <vector>

namespace madron::timbre {
struct Parameters { float transposeCents {}; float formantCents {}; float wetDry { 1.0f }; std::size_t activeVoices { 20 }; float targetFrequencyHz {}; };
class TimbreResynth final : public dsp::AudioProcessor {
public:
    static constexpr std::size_t maximumVoices = GrainResynthesizer::maximumVoices, eventQueueCapacity = 128;
    static constexpr std::array parameterDescriptors {
        dsp::ParameterDescriptor { "timbre.transpose_cents", "Transpose", dsp::ParameterUnit::Cents, -2400.0f, 2400.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "timbre.formant_cents", "Formant", dsp::ParameterUnit::Cents, -2400.0f, 2400.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "timbre.wet_dry", "Wet / Dry", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 1.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "timbre.active_voices", "Active Voices", dsp::ParameterUnit::Choice, 1.0f, 20.0f, 20.0f, dsp::ParameterMapping::Enumerated, dsp::SmoothingPolicy::None, true, true },
        dsp::ParameterDescriptor { "timbre.target_hz", "Target Frequency", dsp::ParameterUnit::Hertz, 0.0f, 12000.0f, 0.0f, dsp::ParameterMapping::Exponential, dsp::SmoothingPolicy::LinearRamp, true, true }
    };
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept override;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override { return GrainResynthesizer::latencySamples; }
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }
    void setParameters(const Parameters& parameters) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept;
    bool enqueuePartial(const PartialEvent& event) noexcept;
private:
    struct Channel { PartialTracker tracker {}; GrainResynthesizer grain {}; dsp::FixedDelay dryDelay {}; float smoothWet { 1.0f }; };
    [[nodiscard]] GrainParameters snapshot() const noexcept;
    void dispatchQueuedEvents() noexcept;
    dsp::ProcessSpec spec_ {}; std::vector<Channel> channels_ {}; std::array<PartialEvent,eventQueueCapacity> eventQueue_ {}; std::atomic<std::size_t> eventWrite_ {0},eventRead_ {0};
    std::atomic<float> transposeCents_ {0.0f},formantCents_ {0.0f},wetDry_ {1.0f},targetFrequencyHz_ {0.0f}; std::atomic<std::size_t> activeVoices_ {20}; bool prepared_ {};
};
} // namespace madron::timbre
