#include "PluginProcessor.h"
#include "PluginEditor.h"

MadronAudioProcessor::MadronAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    state_.setProperty("version", "0.2.0", nullptr);
    state_.setProperty("status", "P02 DSP Engines", nullptr);
}
bool MadronAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const { return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
void MadronAudioProcessor::prepareToPlay(double rate, int block) { const madron::dsp::ProcessSpec spec {rate, static_cast<std::size_t>(block), 2}; prepared_=formant_.prepare(spec)&&cepstral_.prepare(spec)&&timbre_.prepare(spec); }
void MadronAudioProcessor::releaseResources() { formant_.reset(); cepstral_.reset(); timbre_.reset(); prepared_=false; }
void MadronAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    if (!prepared_ || buffer.getNumChannels() != 2) { buffer.clear(); return; }
    const float* inputs[] { buffer.getReadPointer(0), buffer.getReadPointer(1) };
    float* outputs[] { buffer.getWritePointer(0), buffer.getWritePointer(1) };
    const madron::dsp::AudioBufferView input { inputs, 2, static_cast<std::size_t>(buffer.getNumSamples()) };
    const madron::dsp::MutableAudioBufferView output { outputs, 2, static_cast<std::size_t>(buffer.getNumSamples()) };
    switch (diagnosticEngine()) {
        case DiagnosticEngine::Formant: (void) formant_.process(input, output); break;
        case DiagnosticEngine::Cepstral: (void) cepstral_.process(input, output); break;
        case DiagnosticEngine::Timbre: (void) timbre_.process(input, output); break;
        case DiagnosticEngine::Bypass: break;
    }
}
juce::AudioProcessorEditor* MadronAudioProcessor::createEditor() { return new MadronAudioProcessorEditor(*this); }
void MadronAudioProcessor::getStateInformation(juce::MemoryBlock& destination) { state_.setProperty("diagnosticEngine", diagnosticEngine_.load(), nullptr); if(auto xml=state_.createXml()) copyXmlToBinary(*xml,destination); }
void MadronAudioProcessor::setStateInformation(const void* data, int size) { if(auto xml=getXmlFromBinary(data,size)) if(xml->hasTagName(state_.getType())) { state_=juce::ValueTree::fromXml(*xml); diagnosticEngine_.store(static_cast<int>(state_.getProperty("diagnosticEngine", 0))); } }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MadronAudioProcessor(); }
