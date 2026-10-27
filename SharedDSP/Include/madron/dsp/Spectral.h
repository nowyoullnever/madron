#pragma once

#include <complex>
#include <cstddef>
#include <span>
#include <vector>

namespace madron::dsp {

using Complex = std::complex<float>;

class FFTPlan final {
public:
    bool prepare(std::size_t size);
    void transform(std::span<Complex> values, bool inverse) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept { return size_; }

private:
    std::size_t size_ {};
    std::vector<std::size_t> bitReverse_ {};
};

// Streaming, real-valued STFT with preallocated analysis history and OLA output.
// The processed frame is emitted N samples later; `processSample` never allocates.
class StreamingStft final {
public:
    using FrameProcessor = void (*)(void* context, std::span<Complex> spectrum) noexcept;

    bool prepare(std::size_t fftSize, std::size_t overlap);
    void reset() noexcept;
    float processSample(float input, void* context, FrameProcessor processor) noexcept;

    [[nodiscard]] std::span<const Complex> spectrum() const noexcept { return analysisSpectrum_; }
    [[nodiscard]] std::size_t fftSize() const noexcept { return fftSize_; }
    [[nodiscard]] std::size_t hopSize() const noexcept { return hopSize_; }
    [[nodiscard]] std::size_t latencySamples() const noexcept { return fftSize_; }
    [[nodiscard]] bool frameWasProcessed() const noexcept { return frameWasProcessed_; }

private:
    void processFrame(void* context, FrameProcessor processor) noexcept;

    FFTPlan fft_ {};
    std::size_t fftSize_ {};
    std::size_t hopSize_ {};
    std::size_t inputWrite_ {};
    std::size_t samplesSeen_ {};
    std::size_t samplesSinceFrame_ {};
    std::size_t outputTime_ {};
    bool frameWasProcessed_ {};
    std::vector<float> inputRing_ {};
    std::vector<float> window_ {};
    std::vector<Complex> spectrum_ {};
    std::vector<Complex> analysisSpectrum_ {};
    std::vector<float> ola_ {};
    std::vector<float> normalization_ {};
};

class FixedDelay final {
public:
    bool prepare(std::size_t delaySamples);
    void reset() noexcept;
    float process(float input) noexcept;
    [[nodiscard]] std::size_t delaySamples() const noexcept { return delaySamples_; }

private:
    std::vector<float> buffer_ {};
    std::size_t delaySamples_ {};
    std::size_t write_ {};
};

[[nodiscard]] float clampFinite(float value, float low, float high, float fallback = 0.0f) noexcept;
[[nodiscard]] float wrapPhase(float radians) noexcept;

} // namespace madron::dsp
