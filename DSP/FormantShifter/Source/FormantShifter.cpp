#include <madron/formant/FormantShifter.h>

#include <algorithm>
#include <cmath>

namespace madron::formant {
bool FormantShifter::prepare(const dsp::ProcessSpec& spec) {
    prepared_ = dsp::isSupportedStereoOrMono(spec); if (!prepared_) return false; spec_ = spec;
    channels_.clear(); channels_.resize(spec.channelCount);
    for (auto& channel : channels_) {
        if (!channel.stft.prepare(fftSize, overlap) || !channel.dryDelay.prepare(fftSize)) { prepared_ = false; return false; }
        channel.magnitude.assign(fftSize / 2 + 1, 0.0f); channel.envelope.assign(fftSize / 2 + 1, 0.0f);
    }
    reset(); return true;
}
void FormantShifter::reset() noexcept {
    for (auto& channel : channels_) { channel.stft.reset(); channel.dryDelay.reset(); std::fill(channel.magnitude.begin(),channel.magnitude.end(),0.0f); std::fill(channel.envelope.begin(),channel.envelope.end(),0.0f); channel.smoothShiftCents=shiftCents_.load(); channel.smoothWet=wetDry_.load(); }
}
void FormantShifter::setParameters(const Parameters& p) noexcept { shiftCents_.store(dsp::clampFinite(p.shiftCents,-2400.0f,2400.0f)); envelopeWidth_.store(dsp::clampFinite(p.envelopeWidth,1.0f,128.0f,10.0f)); wetDry_.store(dsp::clampFinite(p.wetDry,0.0f,1.0f,1.0f)); }
Parameters FormantShifter::parameters() const noexcept { return {shiftCents_.load(),envelopeWidth_.load(),wetDry_.load()}; }
dsp::ProcessResult FormantShifter::process(const dsp::AudioBufferView& input,const dsp::MutableAudioBufferView& output) noexcept {
    if (!prepared_) return dsp::ProcessResult::NotPrepared;
    if(!input.isValid()||!output.isValid()||input.channelCount!=spec_.channelCount||output.channelCount!=spec_.channelCount||input.sampleCount!=output.sampleCount||input.sampleCount>spec_.maximumBlockSize) return dsp::ProcessResult::InvalidBuffer;
    for(std::size_t sample=0;sample<input.sampleCount;++sample) for(std::size_t index=0;index<spec_.channelCount;++index) {
        auto& channel=channels_[index]; const float source=input.channels[index][sample]; const float dry=channel.dryDelay.process(source); CallbackContext context{this,&channel}; const float wet=channel.stft.processSample(source,&context,&FormantShifter::processSpectrum); channel.smoothWet+=0.0025f*(wetDry_.load()-channel.smoothWet); output.channels[index][sample]=dry+channel.smoothWet*(wet-dry);
    }
    return dsp::ProcessResult::Ok;
}
void FormantShifter::processSpectrum(void* opaque,const std::span<dsp::Complex> spectrum) noexcept { auto& context=*static_cast<CallbackContext*>(opaque); context.owner->shapeSpectrum(*context.channel,spectrum); }
void FormantShifter::shapeSpectrum(Channel& channel,const std::span<dsp::Complex> spectrum) noexcept {
    const std::size_t half=fftSize/2; for(std::size_t bin=0;bin<=half;++bin) channel.magnitude[bin]=std::abs(spectrum[bin]);
    const auto width=std::clamp<std::size_t>(static_cast<std::size_t>(std::lround(envelopeWidth_.load())),1,128); channel.envelope[0]=channel.magnitude[0]; std::size_t previous=0; float previousValue=channel.envelope[0];
    for(std::size_t start=1;start<half;start+=width) { const std::size_t end=std::min(half,start+width); float mean=0.0f,peakValue=-1.0f; std::size_t peak=start; for(std::size_t bin=start;bin<end;++bin) mean+=channel.magnitude[bin]; mean/=static_cast<float>(std::max<std::size_t>(1,end-start)); for(std::size_t bin=start;bin<end;++bin) if(channel.magnitude[bin]>mean&&channel.magnitude[bin]>=peakValue){peakValue=channel.magnitude[bin];peak=bin;} if(peakValue<0.0f){peak=(start+end-1)/2;peakValue=channel.magnitude[peak];} const auto span=std::max<std::size_t>(1,peak-previous); for(std::size_t bin=previous;bin<=peak;++bin){const float t=static_cast<float>(bin-previous)/static_cast<float>(span);channel.envelope[bin]=previousValue+t*(peakValue-previousValue);} previous=peak;previousValue=peakValue; }
    const auto tail=std::max<std::size_t>(1,half-previous); for(std::size_t bin=previous;bin<=half;++bin){const float t=static_cast<float>(bin-previous)/static_cast<float>(tail);channel.envelope[bin]=previousValue+t*(channel.magnitude[half]-previousValue);}
    channel.smoothShiftCents+=0.15f*(shiftCents_.load()-channel.smoothShiftCents); const float shift=std::exp2(channel.smoothShiftCents/1200.0f); const auto highMask=static_cast<std::size_t>(std::max(0.0f,static_cast<float>(half)/2.36f-9.0f));
    for(std::size_t bin=0;bin<=half;++bin){const float source=static_cast<float>(bin)/shift;const auto lower=static_cast<std::size_t>(std::min<float>(static_cast<float>(half),std::floor(source)));const auto upper=std::min(half,lower+1);const float f=source-static_cast<float>(lower);const float shifted=channel.envelope[lower]+f*(channel.envelope[upper]-channel.envelope[lower]);const float detail=channel.magnitude[bin]/std::max(1.0e-8f,channel.envelope[bin]);const float magnitude=(bin<6||bin>highMask)?0.0f:detail*shifted;const float phase=std::atan2(spectrum[bin].imag(),spectrum[bin].real());spectrum[bin]=std::polar(std::isfinite(magnitude)?magnitude:0.0f,phase);if(bin!=0&&bin!=half)spectrum[fftSize-bin]=std::conj(spectrum[bin]);}
}
} // namespace madron::formant
