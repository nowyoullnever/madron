#pragma once

#include <madron/dsp/Spectral.h>

#include <array>
#include <cstddef>
#include <cstdint>

namespace madron::timbre {
enum class PartialEventType : unsigned char { Start, Update, Stop };
struct PartialEvent { float frequencyHz {}; float amplitude {}; std::uint32_t trackId {}; std::size_t voiceIndex {}; std::size_t channelIndex {}; PartialEventType type { PartialEventType::Update }; std::uint64_t sampleTime {}; };

class PartialTracker final {
public:
    static constexpr std::size_t analysisSize = 4096, hopSize = 1024, maximumPeaks = 20;
    bool prepare(double sampleRate);
    void reset() noexcept;
    void push(float sample) noexcept;
    [[nodiscard]] bool pop(PartialEvent& event) noexcept;
private:
    struct Track { float frequency {}, amplitude {}; std::uint32_t id {}; std::uint8_t missed {}; bool active {}; };
    void analyseFrame() noexcept;
    double sampleRate_ {}; dsp::FFTPlan fft_ {}; std::array<float,analysisSize> ring_ {}, window_ {}; std::array<dsp::Complex,analysisSize> spectrum_ {};
    std::array<Track,maximumPeaks> tracks_ {}; std::array<PartialEvent,maximumPeaks * 2> events_ {}; std::size_t write_ {}, seen_ {}, sinceHop_ {}, eventRead_ {}, eventWrite_ {}; std::uint32_t nextTrackId_ { 1 }; std::uint64_t sampleTime_ {};
    float decimationSum_ {}, previousDecimated_ {}, periodHint_ {}; std::size_t decimationCount_ {}; std::uint64_t lastPositiveCross_ {};
};
} // namespace madron::timbre
