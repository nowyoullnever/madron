#pragma once

#include <madron/dsp/AudioBufferView.h>
#include <madron/dsp/Processor.h>

#include <atomic>
#include <cstddef>
#include <vector>

namespace madron::inputb {

enum class Mode : unsigned char { FilteredCopy, DistortedCopy, DelayedCopy, DetunedCopy };

struct Parameters {
    Mode mode { Mode::FilteredCopy };
    float cutoffHz { 1800.0f };
    float resonance { 0.0f };
    float drive { 2.0f };
    float delayMilliseconds { 80.0f };
    float detuneCents { 35.0f };
};

class InputBGenerator final : public dsp::AudioProcessor {
public:
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept override;
    void setParameters(const Parameters& parameters) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept;
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override { return 0; }
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }

private:
    [[nodiscard]] float processSample(std::size_t channel, float input) noexcept;
    [[nodiscard]] float readCubic(const std::vector<float>& buffer, float position) const noexcept;
    dsp::ProcessSpec spec_ {};
    std::size_t delayCapacity_ {};
    std::vector<std::vector<float>> delay_ {};
    std::vector<std::size_t> write_ {};
    std::vector<float> lowpass_ {};
    std::vector<float> detuneRead_ {};
    std::atomic<int> mode_ { static_cast<int>(Mode::FilteredCopy) };
    std::atomic<float> cutoffHz_ { 1800.0f };
    std::atomic<float> resonance_ { 0.0f };
    std::atomic<float> drive_ { 2.0f };
    std::atomic<float> delayMilliseconds_ { 80.0f };
    std::atomic<float> detuneCents_ { 35.0f };
    bool prepared_ {};
};

} // namespace madron::inputb
