#pragma once

#include <madron/dsp/Parameter.h>
#include <madron/dsp/Processor.h>
#include <madron/dsp/Spectral.h>
#include <madron/inputb/InputBGenerator.h>

#include <array>
#include <atomic>
#include <vector>

namespace madron::cepstral {
enum class PhaseMode { Normal, Inverted, AOnly, BOnly };
enum class FFTSize : std::size_t { N512 = 512, N1024 = 1024, N2048 = 2048, N4096 = 4096 };
struct Parameters { FFTSize fftSize { FFTSize::N1024 }; float envelopeBlend {}; float detailBlend {}; float phaseBlend {}; PhaseMode phaseMode { PhaseMode::Normal }; };

class CepstralMorph final : public dsp::AudioProcessor {
public:
    static constexpr std::size_t overlap = 8;
    static constexpr std::array parameterDescriptors {
        dsp::ParameterDescriptor { "cepstral.fft_size", "FFT Size", dsp::ParameterUnit::Choice, 0.0f, 3.0f, 1.0f, dsp::ParameterMapping::Enumerated, dsp::SmoothingPolicy::None, false, true },
        dsp::ParameterDescriptor { "cepstral.envelope_blend", "Envelope Blend", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "cepstral.detail_blend", "Detail Blend", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "cepstral.phase_blend", "Phase Blend", dsp::ParameterUnit::Percent, 0.0f, 1.0f, 0.0f, dsp::ParameterMapping::Linear, dsp::SmoothingPolicy::LinearRamp, true, true },
        dsp::ParameterDescriptor { "cepstral.phase_mode", "Phase Mode", dsp::ParameterUnit::Choice, 0.0f, 3.0f, 0.0f, dsp::ParameterMapping::Enumerated, dsp::SmoothingPolicy::None, true, true }
    };
    bool prepare(const dsp::ProcessSpec& spec) override;
    void reset() noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& inputA, const dsp::MutableAudioBufferView& output) noexcept override;
    dsp::ProcessResult process(const dsp::AudioBufferView& inputA, const dsp::AudioBufferView& inputB, const dsp::MutableAudioBufferView& output) noexcept;
    void setParameters(const Parameters& parameters) noexcept;
    [[nodiscard]] Parameters parameters() const noexcept;
    void setInternalBParameters(const inputb::Parameters& parameters) noexcept { inputB_.setParameters(parameters); }
    [[nodiscard]] inputb::InputBGenerator& internalBGenerator() noexcept { return inputB_; }
    [[nodiscard]] std::size_t getLatencySamples() const noexcept override;
    [[nodiscard]] dsp::ProcessSpec getProcessSpec() const noexcept override { return spec_; }
private:
    struct Channel {
        dsp::StreamingStft a {}, b {};
        std::vector<dsp::Complex> cepA {}, cepB {}, envA {}, envB {}, detailA {}, detailB {};
        float smoothEnvelope {}, smoothDetail {}, smoothPhase {};
    };
    struct Variant { std::size_t size {}; dsp::FFTPlan cepstralFft {}; std::vector<Channel> channels {}; };
    struct CallbackContext { CepstralMorph* owner {}; Variant* variant {}; Channel* channel {}; };
    static void processSpectrum(void* context, std::span<dsp::Complex> spectrum) noexcept;
    void morphSpectrum(Variant& variant, Channel& channel, std::span<dsp::Complex> spectrum) noexcept;
    dsp::ProcessResult processImpl(const dsp::AudioBufferView& inputA, const dsp::AudioBufferView& inputB, const dsp::MutableAudioBufferView& output) noexcept;
    static constexpr std::array<std::size_t,4> sizes_ { 512,1024,2048,4096 };
    dsp::ProcessSpec spec_ {}; std::array<Variant,4> variants_ {};
    inputb::InputBGenerator inputB_ {}; std::vector<std::vector<float>> internalBBuffer_ {}; std::vector<float*> internalBPointers_ {}; std::vector<const float*> internalBConstPointers_ {};
    std::atomic<int> fftSelection_ { 1 }, phaseMode_ { 0 }; std::atomic<float> envelopeBlend_ { 0.0f }, detailBlend_ { 0.0f }, phaseBlend_ { 0.0f };
    bool prepared_ {};
};
} // namespace madron::cepstral
