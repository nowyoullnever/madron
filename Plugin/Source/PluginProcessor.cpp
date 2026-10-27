#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

MadronAudioProcessor::MadronAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true).withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
    state_.setProperty("version", "0.3.0", nullptr);
    state_.setProperty("status", "P03 Random Graph Runtime", nullptr);
}
bool MadronAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const { return layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
void MadronAudioProcessor::prepareToPlay(double rate, int block) { const madron::dsp::ProcessSpec spec {rate, static_cast<std::size_t>(block), 2}; std::lock_guard lock(recipeMutex_); graph_.resetNonRealtime(); dryBuffer_.assign(2,std::vector<float>(static_cast<std::size_t>(block))); dryDelay_.assign(2,{}); for(auto& delay:dryDelay_)delay.prepare(8192); const auto request=madron::random::RecipeRequest{{seed_.load()},static_cast<madron::random::RecipeCategory>(category_.load()),static_cast<std::uint8_t>(intensity_.load()),static_cast<std::uint8_t>(complexity_.load()),static_cast<madron::random::PerformanceMode>(performance_.load()),currentRecipe_.locks}; currentRecipe_=generator_.generate(request, currentRecipe_.graph.nodes.empty()?nullptr:&currentRecipe_); madron::random::ValidationResult error; auto compiled=compiler_.compile(currentRecipe_.graph,{spec,32,8192},&error); prepared_=compiled&&graph_.setInitial(std::move(compiled)); if(prepared_){setLatencySamples(8192);history_.push(currentRecipe_);} }
void MadronAudioProcessor::releaseResources() { std::lock_guard lock(recipeMutex_); graph_.resetNonRealtime(); dryBuffer_.clear();dryDelay_.clear();prepared_=false; }
void MadronAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;
    if (!prepared_ || buffer.getNumChannels() != 2) { buffer.clear(); return; }
    for(std::size_t c=0;c<2;++c)std::copy_n(buffer.getReadPointer(static_cast<int>(c)),static_cast<std::size_t>(buffer.getNumSamples()),dryBuffer_[c].data());
    const float* inputs[] { buffer.getReadPointer(0), buffer.getReadPointer(1) };
    float* outputs[] { buffer.getWritePointer(0), buffer.getWritePointer(1) };
    const madron::dsp::AudioBufferView input { inputs, 2, static_cast<std::size_t>(buffer.getNumSamples()) };
    const madron::dsp::MutableAudioBufferView output { outputs, 2, static_cast<std::size_t>(buffer.getNumSamples()) };
    graph_.setTransitionSamples(static_cast<std::size_t>(getSampleRate()*transitionMilliseconds_.load()/1000.0f));
    if(graph_.process(input, output)!=madron::dsp::ProcessResult::Ok){buffer.clear();return;}
    const float wet=globalWet_.load(std::memory_order_acquire);for(std::size_t c=0;c<2;++c)for(int s=0;s<buffer.getNumSamples();++s){const float dry=dryDelay_[c].process(dryBuffer_[c][static_cast<std::size_t>(s)]);const float value=dry+(buffer.getWritePointer(static_cast<int>(c))[s]-dry)*wet;buffer.getWritePointer(static_cast<int>(c))[s]=std::isfinite(value)?value:0.0f;}
}
juce::AudioProcessorEditor* MadronAudioProcessor::createEditor() { return new MadronAudioProcessorEditor(*this); }
void MadronAudioProcessor::rollRecipe(){std::lock_guard lock(recipeMutex_);const auto next=seed_.fetch_add(0x9e3779b97f4a7c15ull)+0x9e3779b97f4a7c15ull;const auto request=madron::random::RecipeRequest{{next},static_cast<madron::random::RecipeCategory>(category_.load()),static_cast<std::uint8_t>(intensity_.load()),static_cast<std::uint8_t>(complexity_.load()),static_cast<madron::random::PerformanceMode>(performance_.load()),currentRecipe_.locks};auto recipe=generator_.generate(request,currentRecipe_.graph.nodes.empty()?nullptr:&currentRecipe_);if(queueRecipe(recipe)){currentRecipe_=recipe;history_.push(recipe);}}
bool MadronAudioProcessor::queueRecipe(const madron::random::Recipe& recipe){if(!prepared_)return false;madron::random::ValidationResult error;auto graph=compiler_.compile(recipe.graph,{{getSampleRate(),static_cast<std::size_t>(getBlockSize()),2},32,8192},&error);return graph&&graph_.request(std::move(graph));}
juce::String MadronAudioProcessor::graphSummary() const {std::lock_guard lock(recipeMutex_);madron::random::GraphCompiler compiler;madron::random::ValidationResult error;auto view=compiler.compile(currentRecipe_.graph,{{getSampleRate(),static_cast<std::size_t>(std::max(1,getBlockSize())),2},32,8192},&error);return view?juce::String(view->summary()):juce::String("Invalid graph");}
void MadronAudioProcessor::undoRecipe(){std::lock_guard lock(recipeMutex_);if(const auto* recipe=history_.undo())if(queueRecipe(*recipe))currentRecipe_=*recipe;}
void MadronAudioProcessor::redoRecipe(){std::lock_guard lock(recipeMutex_);if(const auto* recipe=history_.redo())if(queueRecipe(*recipe))currentRecipe_=*recipe;}
juce::String MadronAudioProcessor::recipeJson() const {std::lock_guard lock(recipeMutex_);return madron::random::serializeRecipeJson(currentRecipe_);}
bool MadronAudioProcessor::loadRecipeJson(const juce::String& text){std::lock_guard lock(recipeMutex_);std::string error;auto recipe=madron::random::deserializeRecipeJson(text.toStdString(),&error);if(!recipe||!queueRecipe(*recipe))return false;currentRecipe_=*recipe;history_.push(*recipe);seed_.store(recipe->seed.value);category_.store(static_cast<int>(recipe->category));intensity_.store(recipe->intensity);complexity_.store(recipe->complexity);performance_.store(static_cast<int>(recipe->performance));return true;}
void MadronAudioProcessor::favoriteRecipe(){std::lock_guard lock(recipeMutex_);history_.setFavorite(currentRecipe_);}
void MadronAudioProcessor::restoreFavorite(){std::lock_guard lock(recipeMutex_);if(const auto& favorite=history_.favorite())if(queueRecipe(*favorite))currentRecipe_=*favorite;}
void MadronAudioProcessor::getStateInformation(juce::MemoryBlock& destination) {std::lock_guard lock(recipeMutex_);state_.setProperty("recipe",juce::String(madron::random::serializeRecipeJson(currentRecipe_)),nullptr);state_.setProperty("globalWet",globalWet_.load(),nullptr);if(auto xml=state_.createXml())copyXmlToBinary(*xml,destination);}
void MadronAudioProcessor::setStateInformation(const void* data, int size) {if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(state_.getType())){std::lock_guard lock(recipeMutex_);state_=juce::ValueTree::fromXml(*xml);std::string error;auto recipe=madron::random::deserializeRecipeJson(state_.getProperty("recipe").toString().toStdString(),&error);if(recipe&&queueRecipe(*recipe))currentRecipe_=*recipe;globalWet_.store(static_cast<float>(state_.getProperty("globalWet",1.0f)));}}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new MadronAudioProcessor(); }
