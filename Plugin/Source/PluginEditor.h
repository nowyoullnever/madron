#pragma once
#include "PluginProcessor.h"

class MadronAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Button::Listener, private juce::Timer {
public:
    explicit MadronAudioProcessorEditor(MadronAudioProcessor& processor);
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void buttonClicked(juce::Button*) override;
    void timerCallback() override;
    MadronAudioProcessor& processor_;
    juce::Label title_, status_, graphLabel_, seedLabel_;
    juce::ComboBox category_, performance_;
    juce::TextEditor seedInput_, nodeIdInput_, parameterIdInput_;
    juce::Slider intensity_, complexity_, wet_, transition_;
    juce::TextButton roll_{"RANDOM / ROLL"}, undo_{"Undo"}, redo_{"Redo"}, favorite_{"Favorite"}, restoreFavorite_{"Restore Favorite"}, save_{"Save"}, load_{"Load"}, structureLock_{"Structure Lock"}, nodeLock_{"Lock Node"}, parameterLock_{"Lock Parameter"};
    std::unique_ptr<juce::FileChooser> chooser_;
};
