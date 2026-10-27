#pragma once
#include "PluginProcessor.h"
class MadronAudioProcessorEditor final : public juce::AudioProcessorEditor {
public:
    explicit MadronAudioProcessorEditor(MadronAudioProcessor& processor);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    MadronAudioProcessor& processor_;
    juce::Label title_, version_, stateLabel_;
};
