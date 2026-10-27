#include <madron/inputb/InputBGenerator.h>
#include <madron/dsp/Spectral.h>

#include <algorithm>
#include <cmath>
#include <numbers>

namespace madron::inputb {

bool InputBGenerator::prepare(const dsp::ProcessSpec& spec) {
    prepared_ = dsp::isSupportedStereoOrMono(spec);
    if (!prepared_) return false;
    spec_ = spec;
    delayCapacity_ = static_cast<std::size_t>(std::ceil(spec.sampleRate * 1.05)) + 8;
    delay_.assign(spec.channelCount, std::vector<float>(delayCapacity_, 0.0f));
    write_.assign(spec.channelCount, 0);
    lowpass_.assign(spec.channelCount, 0.0f);
    detuneRead_.assign(spec.channelCount, static_cast<float>(delayCapacity_ - 512));
    return true;
}

void InputBGenerator::reset() noexcept {
    for (auto& channel : delay_) std::fill(channel.begin(), channel.end(), 0.0f);
    std::fill(write_.begin(), write_.end(), 0);
    std::fill(lowpass_.begin(), lowpass_.end(), 0.0f);
    std::fill(detuneRead_.begin(), detuneRead_.end(), static_cast<float>(delayCapacity_ - 512));
}

void InputBGenerator::setParameters(const Parameters& parameters) noexcept {
    mode_.store(static_cast<int>(parameters.mode), std::memory_order_release);
    cutoffHz_.store(dsp::clampFinite(parameters.cutoffHz, 20.0f, 18000.0f, 1800.0f), std::memory_order_release);
    resonance_.store(dsp::clampFinite(parameters.resonance, 0.0f, 0.99f), std::memory_order_release);
    drive_.store(dsp::clampFinite(parameters.drive, 0.1f, 20.0f, 2.0f), std::memory_order_release);
    delayMilliseconds_.store(dsp::clampFinite(parameters.delayMilliseconds, 0.0f, 1000.0f, 80.0f), std::memory_order_release);
    detuneCents_.store(dsp::clampFinite(parameters.detuneCents, -2400.0f, 2400.0f, 35.0f), std::memory_order_release);
}

Parameters InputBGenerator::parameters() const noexcept {
    return { static_cast<Mode>(mode_.load(std::memory_order_acquire)), cutoffHz_.load(std::memory_order_acquire), resonance_.load(std::memory_order_acquire), drive_.load(std::memory_order_acquire), delayMilliseconds_.load(std::memory_order_acquire), detuneCents_.load(std::memory_order_acquire) };
}

dsp::ProcessResult InputBGenerator::process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept {
    if (!prepared_) return dsp::ProcessResult::NotPrepared;
    if (!input.isValid() || !output.isValid() || input.channelCount != spec_.channelCount || output.channelCount != spec_.channelCount || input.sampleCount != output.sampleCount || input.sampleCount > spec_.maximumBlockSize) return dsp::ProcessResult::InvalidBuffer;
    for (std::size_t sample = 0; sample < input.sampleCount; ++sample)
        for (std::size_t channel = 0; channel < input.channelCount; ++channel)
            output.channels[channel][sample] = processSample(channel, input.channels[channel][sample]);
    return dsp::ProcessResult::Ok;
}

float InputBGenerator::processSample(const std::size_t channel, const float rawInput) noexcept {
    const float input = std::isfinite(rawInput) ? rawInput : 0.0f;
    auto& buffer = delay_[channel];
    const std::size_t write = write_[channel];
    buffer[write] = input;
    const Parameters p = parameters();
    float output = input;
    if (p.mode == Mode::FilteredCopy) {
        const float alpha = 1.0f - std::exp(-2.0f * std::numbers::pi_v<float> * p.cutoffHz / static_cast<float>(spec_.sampleRate));
        lowpass_[channel] += alpha * (input - lowpass_[channel]);
        output = lowpass_[channel] * (1.0f - 0.35f * p.resonance);
    } else if (p.mode == Mode::DistortedCopy) {
        output = std::tanh(input * p.drive) / std::tanh(p.drive);
    } else if (p.mode == Mode::DelayedCopy) {
        const std::size_t delaySamples = std::min<std::size_t>(delayCapacity_ - 2, static_cast<std::size_t>(p.delayMilliseconds * static_cast<float>(spec_.sampleRate) / 1000.0f));
        output = buffer[(write + delayCapacity_ - delaySamples) % delayCapacity_];
    } else {
        // A moving fractional read head performs resampling; this is an actual pitch/time alteration, not AM modulation.
        output = readCubic(buffer, detuneRead_[channel]);
        const float rate = std::exp2(p.detuneCents / 1200.0f);
        detuneRead_[channel] += rate;
        while (detuneRead_[channel] >= static_cast<float>(delayCapacity_)) detuneRead_[channel] -= static_cast<float>(delayCapacity_);
        while (detuneRead_[channel] < 0.0f) detuneRead_[channel] += static_cast<float>(delayCapacity_);
        const float distance = std::fmod(static_cast<float>(write) - detuneRead_[channel] + static_cast<float>(delayCapacity_), static_cast<float>(delayCapacity_));
        if (distance < 8.0f || distance > static_cast<float>(delayCapacity_ - 8)) detuneRead_[channel] = static_cast<float>((write + delayCapacity_ - 512) % delayCapacity_);
    }
    write_[channel] = (write + 1) % delayCapacity_;
    return std::isfinite(output) ? output : 0.0f;
}

float InputBGenerator::readCubic(const std::vector<float>& buffer, float position) const noexcept {
    const auto wrap = [size = static_cast<int>(buffer.size())](int value) noexcept { value %= size; return value < 0 ? value + size : value; };
    const int base = static_cast<int>(std::floor(position));
    const float fraction = position - std::floor(position);
    const float y0 = buffer[wrap(base - 1)], y1 = buffer[wrap(base)], y2 = buffer[wrap(base + 1)], y3 = buffer[wrap(base + 2)];
    const float a0 = y3 - y2 - y0 + y1;
    const float a1 = y0 - y1 - a0;
    const float a2 = y2 - y0;
    return ((a0 * fraction + a1) * fraction + a2) * fraction + y1;
}

} // namespace madron::inputb
