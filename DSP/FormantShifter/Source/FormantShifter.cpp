#include <madron/formant/FormantShifter.h>

namespace madron::formant {
bool FormantShifter::prepare(const dsp::ProcessSpec& spec) { prepared_ = dsp::isSupportedStereoOrMono(spec); if (prepared_) spec_ = spec; return prepared_; }
void FormantShifter::reset() noexcept {}
void FormantShifter::setParameters(const Parameters& parameters) noexcept { parameters_ = parameters; }
dsp::ProcessResult FormantShifter::process(const dsp::AudioBufferView& input, const dsp::MutableAudioBufferView& output) noexcept {
    if (!prepared_) return dsp::ProcessResult::NotPrepared;
    return dsp::copyAudio(input, output) ? dsp::ProcessResult::Ok : dsp::ProcessResult::InvalidBuffer;
}
} // namespace madron::formant
