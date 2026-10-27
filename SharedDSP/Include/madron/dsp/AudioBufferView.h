#pragma once

#include <algorithm>
#include <cstddef>
#include <cstring>

namespace madron::dsp {

// Non-owning Float32 PCM buffer. The caller owns all channel memory.
struct AudioBufferView {
    const float* const* channels {};
    std::size_t channelCount {};
    std::size_t sampleCount {};

    [[nodiscard]] bool isValid() const noexcept {
        if (channels == nullptr || channelCount == 0) return false;
        for (std::size_t channel = 0; channel < channelCount; ++channel)
            if (channels[channel] == nullptr) return false;
        return true;
    }
};

struct MutableAudioBufferView {
    float* const* channels {};
    std::size_t channelCount {};
    std::size_t sampleCount {};

    [[nodiscard]] bool isValid() const noexcept {
        if (channels == nullptr || channelCount == 0) return false;
        for (std::size_t channel = 0; channel < channelCount; ++channel)
            if (channels[channel] == nullptr) return false;
        return true;
    }

    void clear() const noexcept {
        for (std::size_t channel = 0; channel < channelCount; ++channel)
            std::fill_n(channels[channel], sampleCount, 0.0f);
    }
};

// Supports per-channel in-place operation. Cross-channel overlap is not supported.
inline bool copyAudio(const AudioBufferView input, const MutableAudioBufferView output) noexcept {
    if (!input.isValid() || !output.isValid() || input.channelCount != output.channelCount || input.sampleCount != output.sampleCount)
        return false;
    for (std::size_t channel = 0; channel < input.channelCount; ++channel)
        std::memmove(output.channels[channel], input.channels[channel], input.sampleCount * sizeof(float));
    return true;
}

} // namespace madron::dsp
