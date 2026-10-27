#include "PluginEditor.h"
MadronAudioProcessorEditor::MadronAudioProcessorEditor(MadronAudioProcessor& processor) : AudioProcessorEditor(&processor), processor_(processor) {
    title_.setText("Madron", juce::dontSendNotification); title_.setFont(28.0f); title_.setJustificationType(juce::Justification::centred);
    version_.setText("v0.2.0", juce::dontSendNotification); version_.setJustificationType(juce::Justification::centred);
    stateLabel_.setText("P02 DSP Engines · Diagnostic mode", juce::dontSendNotification); stateLabel_.setJustificationType(juce::Justification::centred);
    engineLabel_.setText("Engine", juce::dontSendNotification);
    engineSelector_.addItem("Bypass", 1); engineSelector_.addItem("Formant", 2); engineSelector_.addItem("Cepstral", 3); engineSelector_.addItem("Timbre", 4); engineSelector_.setSelectedId(static_cast<int>(processor_.diagnosticEngine()) + 1, juce::dontSendNotification);
    engineSelector_.onChange = [this] { processor_.setDiagnosticEngine(static_cast<MadronAudioProcessor::DiagnosticEngine>(engineSelector_.getSelectedId() - 1)); };
    formantLabel_.setText("Formant shift (cents)", juce::dontSendNotification); cepstralLabel_.setText("Cepstral blend", juce::dontSendNotification); timbreLabel_.setText("Timbre transpose (cents)", juce::dontSendNotification);
    formantShift_.setRange(-1200.0, 1200.0, 1.0); formantShift_.setValue(0.0); formantShift_.onValueChange=[this]{processor_.setFormantShiftCents(static_cast<float>(formantShift_.getValue()));};
    cepstralBlend_.setRange(0.0, 1.0, 0.01); cepstralBlend_.setValue(0.0); cepstralBlend_.onValueChange=[this]{processor_.setCepstralEnvelopeBlend(static_cast<float>(cepstralBlend_.getValue()));};
    timbreTranspose_.setRange(-1200.0, 1200.0, 1.0); timbreTranspose_.setValue(0.0); timbreTranspose_.onValueChange=[this]{processor_.setTimbreTransposeCents(static_cast<float>(timbreTranspose_.getValue()));};
    addAndMakeVisible(title_); addAndMakeVisible(version_); addAndMakeVisible(stateLabel_); addAndMakeVisible(engineLabel_); addAndMakeVisible(engineSelector_); addAndMakeVisible(formantLabel_); addAndMakeVisible(formantShift_); addAndMakeVisible(cepstralLabel_); addAndMakeVisible(cepstralBlend_); addAndMakeVisible(timbreLabel_); addAndMakeVisible(timbreTranspose_); setSize(420,380);
}
void MadronAudioProcessorEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff15151a)); g.setColour(juce::Colours::white); g.drawRect(getLocalBounds(),1); }
void MadronAudioProcessorEditor::resized() { auto area=getLocalBounds().reduced(16); title_.setBounds(area.removeFromTop(64)); version_.setBounds(area.removeFromTop(28)); stateLabel_.setBounds(area.removeFromTop(40)); engineLabel_.setBounds(area.removeFromTop(24)); engineSelector_.setBounds(area.removeFromTop(30)); formantLabel_.setBounds(area.removeFromTop(22)); formantShift_.setBounds(area.removeFromTop(34)); cepstralLabel_.setBounds(area.removeFromTop(22)); cepstralBlend_.setBounds(area.removeFromTop(34)); timbreLabel_.setBounds(area.removeFromTop(22)); timbreTranspose_.setBounds(area.removeFromTop(34)); }
