#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <madron/cepstral/CepstralMorph.h>
#include <madron/formant/FormantShifter.h>
#include <madron/timbre/TimbreResynth.h>

#include <atomic>

class MadronAudioProcessor final : public juce::AudioProcessor {
public:
    enum class DiagnosticEngine : int { Bypass = 0, Formant, Cepstral, Timbre };
    MadronAudioProcessor();
    const juce::String getName() const override { return "Madron"; }
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    int getNumPrograms() override { return 1; } int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {} const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    [[nodiscard]] bool isFoundationPrepared() const noexcept { return prepared_; }
    void setDiagnosticEngine(DiagnosticEngine engine) noexcept { diagnosticEngine_.store(static_cast<int>(engine), std::memory_order_release); }
    [[nodiscard]] DiagnosticEngine diagnosticEngine() const noexcept { return static_cast<DiagnosticEngine>(diagnosticEngine_.load(std::memory_order_acquire)); }
    void setFormantShiftCents(float value) noexcept { auto p = formant_.parameters(); p.shiftCents = value; formant_.setParameters(p); }
    void setCepstralEnvelopeBlend(float value) noexcept { auto p = cepstral_.parameters(); p.envelopeBlend = value; p.detailBlend = value; cepstral_.setParameters(p); }
    void setTimbreTransposeCents(float value) noexcept { auto p = timbre_.parameters(); p.transposeCents = value; timbre_.setParameters(p); }
private:
    madron::formant::FormantShifter formant_;
    madron::cepstral::CepstralMorph cepstral_;
    madron::timbre::TimbreResynth timbre_;
    juce::ValueTree state_ { "MadronState" };
    std::atomic<int> diagnosticEngine_ { static_cast<int>(DiagnosticEngine::Bypass) };
    bool prepared_ {};
};
