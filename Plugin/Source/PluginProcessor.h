#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <madron/random/GraphRuntime.h>
#include <madron/random/Recipe.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

class MadronAudioProcessor final : public juce::AudioProcessor {
public:
    MadronAudioProcessor(); ~MadronAudioProcessor() override;
    const juce::String getName() const override { return "Madron"; }
    void prepareToPlay(double sampleRate, int samplesPerBlock) override; void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; } bool acceptsMidi() const override { return false; } bool producesMidi() const override { return false; }
    int getNumPrograms() override { return 1; } int getCurrentProgram() override { return 0; } void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; } void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*, int) override;

    // P03 controls stay public for host automation, JSON presets and renderer integration.
    void rollRecipe(); void setRecipeCategory(madron::random::RecipeCategory value) noexcept { category_.store(static_cast<int>(value)); }
    void setPerformanceMode(madron::random::PerformanceMode value) noexcept { performance_.store(static_cast<int>(value)); }
    void setRecipeSeed(std::uint64_t value) noexcept { seed_.store(value ? value : 1); }
    void setIntensity(float value) noexcept; void setComplexity(float value) noexcept; void setGlobalWet(float value) noexcept;
    [[nodiscard]] float globalWet() const noexcept; void setTransitionMilliseconds(float value) noexcept;
    [[nodiscard]] bool isRecipeActive() const noexcept { return recipeActive_.load(std::memory_order_acquire); }
    void setThemeDark(bool dark) noexcept { themeDark_.store(dark, std::memory_order_release); }
    [[nodiscard]] bool isThemeDark() const noexcept { return themeDark_.load(std::memory_order_acquire); }
    [[nodiscard]] juce::String graphSummary() const; [[nodiscard]] juce::String recipeJson() const;
    // Development/debugger-only snapshot. It is intentionally not rendered by
    // the VST editor and does not allocate or log from the audio callback.
    [[nodiscard]] madron::random::GraphTransition::Diagnostics graphDiagnostics() const noexcept { return graph_.diagnostics(); }
    bool loadRecipeJson(const juce::String& text); void undoRecipe(); void redoRecipe(); void favoriteRecipe(); void restoreFavorite();
    void toggleStructureLock(); bool toggleNodeLock(madron::random::NodeID id); bool toggleParameterLock(const std::string& id);
private:
    struct CompileRequest { madron::random::Recipe recipe {}; madron::dsp::ProcessSpec spec {}; std::uint64_t generation {}; };
    void workerLoop(std::stop_token); bool enqueueRecipe(const madron::random::Recipe& recipe); void applyRecipeState(const madron::random::Recipe&) noexcept;
    static constexpr std::size_t fixedLatencySamples_ = 8192;
    madron::random::GraphCompiler compiler_ {}; madron::random::GraphTransition graph_ {}; madron::random::RecipeGenerator generator_ {}; madron::random::RecipeHistory history_ {};
    madron::random::Recipe currentRecipe_ {}; std::optional<madron::random::Recipe> restoredRecipe_ {};
    mutable std::mutex recipeMutex_ {}; std::mutex workMutex_ {}, graphLifecycleMutex_ {}; std::condition_variable_any workSignal_ {}; std::optional<CompileRequest> pendingCompile_ {}; std::jthread worker_ {}; std::atomic<std::uint64_t> generation_ { 0 };
    juce::ValueTree state_ { "MadronState" }; juce::AudioParameterFloat* mixParameter_ {};
    // -1 is the UI's deterministic Auto Category mode; explicit API/CLI calls
    // retain their requested category.
    std::atomic<int> category_ { -1 }, performance_ { static_cast<int>(madron::random::PerformanceMode::Balanced) };
    std::atomic<int> lastAutoCategory_ { -1 };
    std::atomic<float> intensity_ { 50.0f }, complexity_ { 50.0f }, transitionMilliseconds_ { 100.0f };
    std::atomic<std::uint64_t> seed_ { 0x4d414452ull }; std::atomic<bool> prepared_ { false }, recipeActive_ { false }, themeDark_ { false };
    std::vector<std::vector<float>> dryBuffer_ {}; std::vector<madron::dsp::FixedDelay> dryDelay_ {}; float smoothedWet_ { 0.5f };
};
