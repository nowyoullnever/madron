#pragma once

#include "AudioBufferView.h"

#include <cstddef>

namespace madron::dsp {

struct ProcessSpec {
    double sampleRate {};
    std::size_t maximumBlockSize {};
    std::size_t channelCount {};
};

enum class ProcessResult { Ok, InvalidBuffer, UnsupportedConfiguration, NotPrepared };

class AudioProcessor {
public:
    virtual ~AudioProcessor() = default;
    virtual bool prepare(const ProcessSpec& spec) = 0;
    virtual void reset() noexcept = 0;
    virtual ProcessResult process(const AudioBufferView& input, const MutableAudioBufferView& output) noexcept = 0;
    [[nodiscard]] virtual std::size_t getLatencySamples() const noexcept = 0;
    [[nodiscard]] virtual ProcessSpec getProcessSpec() const noexcept = 0;
};

inline bool isSupportedStereoOrMono(const ProcessSpec& spec) noexcept {
    return (spec.sampleRate == 44100.0 || spec.sampleRate == 48000.0 || spec.sampleRate == 96000.0)
        && spec.maximumBlockSize > 0 && (spec.channelCount == 1 || spec.channelCount == 2);
}

} // namespace madron::dsp
