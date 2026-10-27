#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <madron/cepstral/CepstralMorph.h>
#include <madron/formant/FormantShifter.h>
#include <madron/timbre/TimbreResynth.h>

class MadronAudioProcessor final : public juce::AudioProcessor {
public:
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
private:
    madron::formant::FormantShifter formant_;
    madron::cepstral::CepstralMorph cepstral_;
    madron::timbre::TimbreResynth timbre_;
    juce::ValueTree state_ { "MadronState" };
    bool prepared_ {};
};
