#pragma once

#include <madron/dsp/AudioBufferView.h>
#include <madron/dsp/Processor.h>
#include <madron/dsp/Spectral.h>
#include <madron/random/Graph.h>

#include <array>
#include <atomic>
#include <memory>
#include <string>
#include <vector>

namespace madron::random {

struct GraphCompileOptions { dsp::ProcessSpec spec {}; std::uint16_t maximumBuffers { 32 }; std::size_t fixedOutputLatency {}; };

class ExecutableGraph final {
public:
    ExecutableGraph() = default;
    struct Buffer;
    struct InputBinding;
    struct Node;
    ~ExecutableGraph();
    ExecutableGraph(const ExecutableGraph&) = delete;
    ExecutableGraph& operator=(const ExecutableGraph&) = delete;
    [[nodiscard]] dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept;
    [[nodiscard]] std::size_t latencySamples() const noexcept { return latencySamples_; }
    [[nodiscard]] std::size_t estimatedBytes() const noexcept { return estimatedBytes_; }
    [[nodiscard]] const std::string& summary() const noexcept { return summary_; }
    [[nodiscard]] const GraphDescription& description() const noexcept { return description_; }
    [[nodiscard]] dsp::ProcessSpec processSpec() const noexcept { return spec_; }
private:
    friend class GraphCompiler;
    GraphDescription description_ {};
    dsp::ProcessSpec spec_ {};
    std::vector<Buffer> buffers_ {};
    std::vector<Node> nodes_ {};
    std::vector<dsp::FixedDelay> outputDelay_ {};
    std::size_t latencySamples_ {};
    std::size_t estimatedBytes_ {};
    std::string summary_ {};
};

class GraphCompiler final {
public:
    [[nodiscard]] std::unique_ptr<ExecutableGraph> compile(const GraphDescription& graph, const GraphCompileOptions& options, ValidationResult* error = nullptr) const;
};

// The GUI/non-audio thread owns request()/collectRetired(). process() only
// exchanges prebuilt raw pointers and runs already prepared graphs.
class GraphTransition final {
public:
    struct Diagnostics {
        std::uint64_t createdGraphs {}, destroyedGraphs {}, retirementQueueSaturations {};
        std::size_t liveGraphs {}, pendingGraphs {}, retiredGraphs {}, peakRetainedGraphs {};
        std::size_t estimatedActiveBytes {}, estimatedTotalBytes {}, peakEstimatedBytes {};
    };
    GraphTransition() = default;
    ~GraphTransition();
    GraphTransition(const GraphTransition&) = delete;
    GraphTransition& operator=(const GraphTransition&) = delete;
    void resetNonRealtime() noexcept;
    bool setInitial(std::unique_ptr<ExecutableGraph> graph) noexcept;
    bool request(std::unique_ptr<ExecutableGraph> graph) noexcept;
    void setTransitionSamples(std::size_t samples) noexcept { transitionSamples_.store(samples < 1 ? 1 : samples, std::memory_order_release); }
    [[nodiscard]] dsp::ProcessResult process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept;
    void collectRetired() noexcept;
    [[nodiscard]] std::size_t latencySamples() const noexcept;
    [[nodiscard]] Diagnostics diagnostics() const noexcept;
private:
    [[nodiscard]] bool canRetireFromAudio() const noexcept;
    void updateAudioDiagnostics() noexcept;
    bool retireFromAudio(ExecutableGraph* graph) noexcept;
    static constexpr std::size_t retirementCapacity_ = 8;
    ExecutableGraph* active_ {};
    ExecutableGraph* incoming_ {};
    // Audio-thread-owned overflow slot. It never deletes; it retains a graph
    // until the bounded retirement ring has room for a safe replacement.
    ExecutableGraph* heldPending_ {};
    std::atomic<ExecutableGraph*> pending_ { nullptr };
    std::array<ExecutableGraph*, retirementCapacity_> retired_ {};
    std::atomic<std::size_t> retireWrite_ { 0 }, retireRead_ { 0 };
    std::atomic<std::uint64_t> createdGraphs_ { 0 }, destroyedGraphs_ { 0 }, retirementQueueSaturations_ { 0 };
    std::atomic<std::size_t> liveGraphs_ { 0 }, activeBytes_ { 0 }, totalBytes_ { 0 }, peakRetainedGraphs_ { 0 }, peakEstimatedBytes_ { 0 };
    std::atomic<std::size_t> transitionSamples_ { 4800 };
    std::size_t fadePosition_ {};
    std::vector<std::vector<float>> oldBuffer_ {}, newBuffer_ {};
};

} // namespace madron::random
