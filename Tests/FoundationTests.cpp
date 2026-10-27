#include <madron/cepstral/CepstralMorph.h>
#include <madron/formant/FormantShifter.h>
#include <madron/random/Graph.h>
#include <madron/timbre/TimbreResynth.h>

#include <array>
#include <cassert>
#include <iostream>

namespace {
madron::dsp::ProcessSpec spec(double rate=48000.0, std::size_t block=17) { return {rate,block,2}; }
template<typename Processor> void verifyPassThrough(Processor& processor) {
    assert(processor.prepare(spec()));
    std::array<float,17> left{},right{},outLeft{},outRight{};
    const float* silentChannels[]{left.data(),right.data()}; float* silentOutput[]{outLeft.data(),outRight.data()};
    assert(processor.process({silentChannels,2,left.size()},{silentOutput,2,left.size()})==madron::dsp::ProcessResult::Ok);
    assert(outLeft == left && outRight == right);
    for(std::size_t i=0;i<left.size();++i) { left[i]=static_cast<float>(i)*0.03125f; right[i]=-left[i]; }
    const float* inputChannels[]{left.data(),right.data()}; float* outputChannels[]{outLeft.data(),outRight.data()};
    assert(processor.process({inputChannels,2,left.size()},{outputChannels,2,left.size()})==madron::dsp::ProcessResult::Ok);
    assert(left==outLeft && right==outRight);
    processor.reset(); assert(processor.prepare(spec(96000.0,64)));
    std::array<float,64> largeLeft{},largeRight{},largeOutLeft{},largeOutRight{};
    for(std::size_t i=0;i<largeLeft.size();++i) { largeLeft[i]=static_cast<float>(i)*0.0078125f; largeRight[i]=-largeLeft[i]; }
    const float* largeInput[]{largeLeft.data(),largeRight.data()}; float* largeOutput[]{largeOutLeft.data(),largeOutRight.data()};
    assert(processor.process({largeInput,2,largeLeft.size()},{largeOutput,2,largeLeft.size()})==madron::dsp::ProcessResult::Ok);
    assert(largeLeft==largeOutLeft && largeRight==largeOutRight);
}
void graphTests() {
    using namespace madron::random;
    GraphDescription valid; valid.nodes={{1,NodeType::Input},{2,NodeType::FormantShifter},{3,NodeType::Output}}; valid.connections={{1,Port::AudioOut,2,Port::MainIn},{2,Port::AudioOut,3,Port::MainIn}}; assert(validate(valid));
    GraphDescription cycle; cycle.nodes={{1,NodeType::FormantShifter},{2,NodeType::TimbreResynth}}; cycle.connections={{1,Port::AudioOut,2,Port::MainIn},{2,Port::AudioOut,1,Port::MainIn}}; assert(validate(cycle).error==GraphError::Cycle);
    auto missing=valid; missing.connections={{1,Port::AudioOut,3,Port::MainIn}}; assert(validate(missing).error==GraphError::MissingRequiredInput);
    GraphDescription duplicate; duplicate.nodes={{1,NodeType::Input},{1,NodeType::Output}}; assert(validate(duplicate).error==GraphError::DuplicateNode);
    GraphDescription invalidPort; invalidPort.nodes={{1,NodeType::Input},{2,NodeType::Output}}; invalidPort.connections={{2,Port::AudioOut,1,Port::MainIn}}; assert(validate(invalidPort).error==GraphError::InvalidSourcePort);
    auto invalidBus=valid; invalidBus.connections[0].sourceChannels=3; assert(validate(invalidBus).error==GraphError::InvalidAudioBus);
    GraphDescription multi; multi.nodes={{1,NodeType::Input},{2,NodeType::FormantShifter},{3,NodeType::FormantShifter},{4,NodeType::Merge},{5,NodeType::Output}}; multi.connections={{1,Port::AudioOut,2,Port::MainIn},{1,Port::AudioOut,3,Port::MainIn},{2,Port::AudioOut,4,Port::MainIn},{3,Port::AudioOut,4,Port::SecondaryIn},{4,Port::AudioOut,5,Port::MainIn}}; assert(validate(multi));
}
}
int main() { madron::formant::FormantShifter formant; madron::cepstral::CepstralMorph cepstral; madron::timbre::TimbreResynth timbre; verifyPassThrough(formant); verifyPassThrough(cepstral); verifyPassThrough(timbre); graphTests(); std::cout << "madron foundation tests passed\n"; }
