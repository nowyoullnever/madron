#include <madron/dsp/Spectral.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace madron::dsp {
namespace {
[[nodiscard]] bool isPowerOfTwo(const std::size_t value) noexcept { return value >= 2 && (value & (value - 1)) == 0; }
constexpr float pi = std::numbers::pi_v<float>;
}

bool FFTPlan::prepare(const std::size_t size) {
    if (!isPowerOfTwo(size)) return false;
    size_ = size;
    bitReverse_.resize(size_);
    std::size_t bits = 0;
    for (std::size_t n = size_; n > 1; n >>= 1) ++bits;
    for (std::size_t index = 0; index < size_; ++index) {
        std::size_t reversed = 0;
        for (std::size_t bit = 0; bit < bits; ++bit) reversed = (reversed << 1U) | ((index >> bit) & 1U);
        bitReverse_[index] = reversed;
    }
    return true;
}

void FFTPlan::transform(const std::span<Complex> values, const bool inverse) const noexcept {
    if (values.size() != size_ || size_ == 0) return;
    for (std::size_t index = 0; index < size_; ++index)
        if (index < bitReverse_[index]) std::swap(values[index], values[bitReverse_[index]]);
    for (std::size_t width = 2; width <= size_; width <<= 1U) {
        const float angle = (inverse ? 2.0f : -2.0f) * pi / static_cast<float>(width);
        const Complex step { std::cos(angle), std::sin(angle) };
        for (std::size_t begin = 0; begin < size_; begin += width) {
            Complex twiddle { 1.0f, 0.0f };
            const std::size_t half = width >> 1U;
            for (std::size_t offset = 0; offset < half; ++offset) {
                const Complex even = values[begin + offset];
                const Complex odd = values[begin + offset + half] * twiddle;
                values[begin + offset] = even + odd;
                values[begin + offset + half] = even - odd;
                twiddle *= step;
            }
        }
    }
    if (inverse) {
        const float inverseSize = 1.0f / static_cast<float>(size_);
        for (auto& value : values) value *= inverseSize;
    }
}

bool StreamingStft::prepare(const std::size_t fftSize, const std::size_t overlap) {
    if (!isPowerOfTwo(fftSize) || overlap == 0 || fftSize % overlap != 0 || !fft_.prepare(fftSize)) return false;
    fftSize_ = fftSize;
    hopSize_ = fftSize / overlap;
    inputRing_.assign(fftSize_, 0.0f);
    window_.resize(fftSize_);
    for (std::size_t index = 0; index < fftSize_; ++index) {
        // sqrt-Hann permits exact OLA normalization with arbitrary supported overlap.
        const float hann = 0.5f - 0.5f * std::cos((2.0f * pi * static_cast<float>(index)) / static_cast<float>(fftSize_));
        window_[index] = std::sqrt(std::max(0.0f, hann));
    }
    spectrum_.assign(fftSize_, {});
    analysisSpectrum_.assign(fftSize_, {});
    const std::size_t outputCapacity = fftSize_ * 8;
    ola_.assign(outputCapacity, 0.0f);
    normalization_.assign(outputCapacity, 0.0f);
    reset();
    return true;
}

void StreamingStft::reset() noexcept {
    std::fill(inputRing_.begin(), inputRing_.end(), 0.0f);
    std::fill(spectrum_.begin(), spectrum_.end(), Complex {});
    std::fill(analysisSpectrum_.begin(), analysisSpectrum_.end(), Complex {});
    std::fill(ola_.begin(), ola_.end(), 0.0f);
    std::fill(normalization_.begin(), normalization_.end(), 0.0f);
    inputWrite_ = samplesSeen_ = samplesSinceFrame_ = outputTime_ = 0;
    frameWasProcessed_ = false;
}

float StreamingStft::processSample(const float input, void* const context, const FrameProcessor processor) noexcept {
    if (fftSize_ == 0) return 0.0f;
    const std::size_t outputIndex = outputTime_ % ola_.size();
    const float denominator = normalization_[outputIndex];
    const float output = denominator > 1.0e-12f ? ola_[outputIndex] / denominator : 0.0f;
    ola_[outputIndex] = 0.0f;
    normalization_[outputIndex] = 0.0f;
    ++outputTime_;

    inputRing_[inputWrite_] = std::isfinite(input) ? input : 0.0f;
    inputWrite_ = (inputWrite_ + 1) % fftSize_;
    ++samplesSeen_;
    ++samplesSinceFrame_;
    frameWasProcessed_ = false;
    if (samplesSeen_ >= fftSize_ && samplesSinceFrame_ >= hopSize_) {
        samplesSinceFrame_ = 0;
        processFrame(context, processor);
        frameWasProcessed_ = true;
    }
    return output;
}

void StreamingStft::processFrame(void* const context, const FrameProcessor processor) noexcept {
    for (std::size_t index = 0; index < fftSize_; ++index)
        spectrum_[index] = Complex { inputRing_[(inputWrite_ + index) % fftSize_] * window_[index], 0.0f };
    fft_.transform(spectrum_, false);
    if (processor != nullptr) processor(context, spectrum_);
    for (auto& bin : spectrum_)
        if (!std::isfinite(bin.real()) || !std::isfinite(bin.imag())) bin = {};
    std::copy(spectrum_.begin(), spectrum_.end(), analysisSpectrum_.begin());
    fft_.transform(spectrum_, true);
    for (std::size_t index = 0; index < fftSize_; ++index) {
        const std::size_t outputIndex = (outputTime_ + index) % ola_.size();
        const float window = window_[index];
        ola_[outputIndex] += spectrum_[index].real() * window;
        normalization_[outputIndex] += window * window;
    }
}

bool FixedDelay::prepare(const std::size_t delaySamples) {
    delaySamples_ = delaySamples;
    buffer_.assign(std::max<std::size_t>(1, delaySamples_), 0.0f);
    write_ = 0;
    return true;
}

void FixedDelay::reset() noexcept { std::fill(buffer_.begin(), buffer_.end(), 0.0f); write_ = 0; }

float FixedDelay::process(const float input) noexcept {
    if (delaySamples_ == 0) return input;
    const float output = buffer_[write_];
    buffer_[write_] = std::isfinite(input) ? input : 0.0f;
    write_ = (write_ + 1) % buffer_.size();
    return output;
}

float clampFinite(const float value, const float low, const float high, const float fallback) noexcept {
    if (!std::isfinite(value)) return fallback;
    return std::clamp(value, low, high);
}

float wrapPhase(float radians) noexcept {
    if (!std::isfinite(radians)) return 0.0f;
    while (radians > pi) radians -= 2.0f * pi;
    while (radians < -pi) radians += 2.0f * pi;
    return radians;
}

} // namespace madron::dsp
