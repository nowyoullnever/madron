#include "PluginEditor.h"
MadronAudioProcessorEditor::MadronAudioProcessorEditor(MadronAudioProcessor& processor) : AudioProcessorEditor(&processor), processor_(processor) {
    title_.setText("Madron", juce::dontSendNotification); title_.setFont(28.0f); title_.setJustificationType(juce::Justification::centred);
    version_.setText("v0.1.0", juce::dontSendNotification); version_.setJustificationType(juce::Justification::centred);
    stateLabel_.setText("Foundation Build · Audio Engine ready", juce::dontSendNotification); stateLabel_.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(title_); addAndMakeVisible(version_); addAndMakeVisible(stateLabel_); setSize(420,180);
}
void MadronAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff15151a)); g.setColour(juce::Colours::white); g.drawRect(getLocalBounds(),1); }
void MadronAudioProcessorEditor::resized() { auto area=getLocalBounds().reduced(16); title_.setBounds(area.removeFromTop(64)); version_.setBounds(area.removeFromTop(28)); stateLabel_.setBounds(area.removeFromTop(40)); }
