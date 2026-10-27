#include <madron/timbre/TimbreResynth.h>
namespace madron::timbre {
bool TimbreResynth::prepare(const dsp::ProcessSpec& spec) { prepared_ = dsp::isSupportedStereoOrMono(spec); if (prepared_) spec_=spec; reset(); return prepared_; }
void TimbreResynth::reset() noexcept { eventRead_.store(0,std::memory_order_relaxed); eventWrite_.store(0,std::memory_order_relaxed); activePartials_.fill({}); }
void TimbreResynth::setParameters(const Parameters& p) noexcept { parameters_=p; if(parameters_.activeVoices>maximumVoices) parameters_.activeVoices=maximumVoices; }
bool TimbreResynth::enqueuePartial(const PartialEvent& event) noexcept {
    if(event.voiceIndex>=maximumVoices) return false;
    const auto write=eventWrite_.load(std::memory_order_relaxed);
    const auto next=(write+1)%eventQueueCapacity;
    if(next==eventRead_.load(std::memory_order_acquire)) return false;
    eventQueue_[write]=event;
    eventWrite_.store(next,std::memory_order_release);
    return true;
}
bool TimbreResynth::dequeuePartial(PartialEvent& event) noexcept {
    const auto read=eventRead_.load(std::memory_order_relaxed);
    if(read==eventWrite_.load(std::memory_order_acquire)) return false;
    event=eventQueue_[read];
    eventRead_.store((read+1)%eventQueueCapacity,std::memory_order_release);
    return true;
}
dsp::ProcessResult TimbreResynth::process(const dsp::AudioBufferView& in, const dsp::MutableAudioBufferView& out) noexcept {
    if(!prepared_) return dsp::ProcessResult::NotPrepared;
    PartialEvent event {};
    while(dequeuePartial(event)) activePartials_[event.voiceIndex]=event;
    return dsp::copyAudio(in,out)?dsp::ProcessResult::Ok:dsp::ProcessResult::InvalidBuffer;
}
} // namespace madron::timbre
