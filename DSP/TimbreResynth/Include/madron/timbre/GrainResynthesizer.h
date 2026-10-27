#pragma once

#include <madron/timbre/PartialTracker.h>

#include <array>
#include <cstddef>
#include <vector>

namespace madron::timbre {
struct GrainParameters { float transposeCents {}; float formantCents {}; float targetFrequencyHz {}; std::size_t activeVoices { 20 }; };
class GrainResynthesizer final {
public:
    static constexpr std::size_t maximumVoices = 20, latencySamples = 2048;
    bool prepare(double sampleRate);
    void reset() noexcept;
    void applyEvent(const PartialEvent& event) noexcept;
    float processSample(float input, const GrainParameters& parameters) noexcept;
private:
    struct Voice { std::uint32_t trackId {}; float frequency {}, amplitude {}, smoothAmplitude {}, samplesToMark {}; bool active {}; };
    void scheduleGrain(Voice& voice, const GrainParameters& parameters) noexcept;
    [[nodiscard]] float cubicRead(float position) const noexcept;
    double sampleRate_ {}; std::vector<float> inputRing_ {}, ola_ {}, norm_ {}; std::array<Voice,maximumVoices> voices_ {}; std::size_t inputWrite_ {}, outputTime_ {};
};
} // namespace madron::timbre
