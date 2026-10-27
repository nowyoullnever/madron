#pragma once
#include "PluginProcessor.h"
class MadronAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit MadronAudioProcessorEditor(MadronAudioProcessor& processor);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    MadronAudioProcessor& processor_;
    juce::Label title_, version_, stateLabel_, engineLabel_, formantLabel_, cepstralLabel_, timbreLabel_;
    juce::ComboBox engineSelector_;
    juce::Slider formantShift_, cepstralBlend_, timbreTranspose_;
};
