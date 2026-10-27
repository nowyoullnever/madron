#include <madron/cepstral/CepstralMorph.h>
namespace madron::cepstral {
bool CepstralMorph::prepare(const dsp::ProcessSpec& spec) { prepared_ = dsp::isSupportedStereoOrMono(spec); if (prepared_) spec_ = spec; return prepared_; }
void CepstralMorph::reset() noexcept {}
dsp::ProcessResult CepstralMorph::process(const dsp::AudioBufferView& inputA, const dsp::MutableAudioBufferView& output) noexcept { if (!prepared_) return dsp::ProcessResult::NotPrepared; return dsp::copyAudio(inputA,output) ? dsp::ProcessResult::Ok : dsp::ProcessResult::InvalidBuffer; }
dsp::ProcessResult CepstralMorph::process(const dsp::AudioBufferView& inputA, const dsp::AudioBufferView& inputB, const dsp::MutableAudioBufferView& output) noexcept { if (!inputB.isValid() || inputB.channelCount != inputA.channelCount || inputB.sampleCount != inputA.sampleCount) return dsp::ProcessResult::InvalidBuffer; return process(inputA, output); }
} // namespace madron::cepstral
