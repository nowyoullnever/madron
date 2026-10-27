#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <madron/random/GraphRuntime.h>
#include <madron/random/Recipe.h>

#include <atomic>
#include <algorithm>
#include <mutex>
#include <vector>

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
    void rollRecipe();
    void setRecipeCategory(madron::random::RecipeCategory value) noexcept { category_.store(static_cast<int>(value)); }
    void setPerformanceMode(madron::random::PerformanceMode value) noexcept { performance_.store(static_cast<int>(value)); }
    void setRecipeSeed(std::uint64_t value) noexcept { seed_.store(value ? value : 1); }
    void setIntensity(float value) noexcept { intensity_.store(std::clamp(value, 0.0f, 100.0f)); }
    void setComplexity(float value) noexcept { complexity_.store(std::clamp(value, 0.0f, 100.0f)); }
    void setGlobalWet(float value) noexcept { globalWet_.store(std::clamp(value, 0.0f, 1.0f)); }
    void setTransitionMilliseconds(float value) noexcept { transitionMilliseconds_.store(std::clamp(value, 50.0f, 200.0f)); }
    [[nodiscard]] std::uint64_t recipeSeed() const noexcept { return seed_.load(); }
    [[nodiscard]] juce::String graphSummary() const;
    void undoRecipe(); void redoRecipe();
    [[nodiscard]] juce::String recipeJson() const;
    bool loadRecipeJson(const juce::String& text);
    void favoriteRecipe(); void restoreFavorite();
    void collectGraphRetirement() noexcept { graph_.collectRetired(); }
    [[nodiscard]] int graphLatencySamples() const noexcept { return static_cast<int>(graph_.latencySamples()); }
private:
    bool queueRecipe(const madron::random::Recipe& recipe);
    madron::random::GraphCompiler compiler_ {};
    madron::random::GraphTransition graph_ {};
    madron::random::RecipeGenerator generator_ {};
    madron::random::RecipeHistory history_ {};
    madron::random::Recipe currentRecipe_ {};
    mutable std::mutex recipeMutex_ {};
    juce::ValueTree state_ { "MadronState" };
    std::atomic<int> diagnosticEngine_ { static_cast<int>(DiagnosticEngine::Bypass) };
    std::atomic<int> category_ { static_cast<int>(madron::random::RecipeCategory::TextureMutation) }, performance_ { static_cast<int>(madron::random::PerformanceMode::Balanced) };
    std::atomic<float> intensity_ { 50.0f }, complexity_ { 50.0f }, globalWet_ { 1.0f }, transitionMilliseconds_ { 100.0f };
    std::atomic<std::uint64_t> seed_ { 0x4d414452ull };
    std::vector<std::vector<float>> dryBuffer_ {};
    std::vector<madron::dsp::FixedDelay> dryDelay_ {};
    bool prepared_ {};
};
