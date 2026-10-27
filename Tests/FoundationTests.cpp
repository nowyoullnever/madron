#include <madron/cepstral/CepstralMorph.h>
#include <madron/formant/FormantShifter.h>
#include <madron/inputb/InputBGenerator.h>
#include <madron/random/Graph.h>
#include <madron/random/GraphRuntime.h>
#include <madron/random/Recipe.h>
#include <madron/timbre/PartialTracker.h>
#include <madron/timbre/TimbreResynth.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <numbers>
#include <string_view>
#include <vector>

namespace {
int failures = 0;
void require(const bool condition, const std::string_view message) { if (!condition) { ++failures; std::cerr << "FAIL: " << message << '\n'; } }
madron::dsp::ProcessSpec spec(std::size_t block = 73) { return { 48000.0, block, 2 }; }
bool finite(const std::vector<float>& signal) { return std::all_of(signal.begin(), signal.end(), [](float x) { return std::isfinite(x); }); }
float rms(const std::vector<float>& signal, std::size_t begin = 0) { double sum = 0.0; for (std::size_t i = begin; i < signal.size(); ++i) sum += static_cast<double>(signal[i]) * signal[i]; return static_cast<float>(std::sqrt(sum / std::max<std::size_t>(1, signal.size() - begin))); }
std::vector<float> tone(const std::size_t samples, const float frequency, const float harmonic = 0.0f) { std::vector<float> output(samples); for (std::size_t i=0;i<samples;++i) { const float t=static_cast<float>(i)/48000.0f; output[i]=0.65f*std::sin(2.0f*std::numbers::pi_v<float>*frequency*t)+harmonic*std::sin(2.0f*std::numbers::pi_v<float>*frequency*4.0f*t); } return output; }
template<class Processor> std::vector<float> renderMonoStereo(Processor& processor, const std::vector<float>& a, const std::vector<float>* b, const std::size_t block) {
    std::vector<float> out(a.size()), right(a.size()), outRight(a.size()); for(std::size_t i=0;i<a.size();++i)right[i]=a[i]*0.7f;
    for(std::size_t offset=0;offset<a.size();offset+=block){const auto n=std::min(block,a.size()-offset);const float* in[]{a.data()+offset,right.data()+offset};float* o[]{out.data()+offset,outRight.data()+offset};if constexpr (requires { processor.process(madron::dsp::AudioBufferView{}, madron::dsp::AudioBufferView{}, madron::dsp::MutableAudioBufferView{}); }) {std::vector<float> br; const float* bi[2]{}; if(b){br.resize(n);for(std::size_t i=0;i<n;++i)br[i]=(*b)[offset+i];bi[0]=br.data();bi[1]=br.data();} else {bi[0]=in[0];bi[1]=in[1];} const auto r=processor.process({in,2,n},{bi,2,n},{o,2,n}); if(r!=madron::dsp::ProcessResult::Ok)std::cerr<<"two input result "<<static_cast<int>(r)<<" n="<<n<<" max="<<processor.getProcessSpec().maximumBlockSize<<'\n';require(r==madron::dsp::ProcessResult::Ok,"two-input process succeeds");} else {const auto r=processor.process({in,2,n},{o,2,n});if(r!=madron::dsp::ProcessResult::Ok)std::cerr<<"one input result "<<static_cast<int>(r)<<" n="<<n<<" max="<<processor.getProcessSpec().maximumBlockSize<<'\n';require(r==madron::dsp::ProcessResult::Ok,"process succeeds");}}
    return out;
}
void sharedAndGraphTests() {
    const madron::dsp::ParameterDescriptor exponential { "test", "test", madron::dsp::ParameterUnit::Hertz, 20.0f, 20000.0f, 20.0f, madron::dsp::ParameterMapping::Exponential };
    const madron::dsp::ParameterDescriptor enumerated { "test", "test", madron::dsp::ParameterUnit::Choice, 0.0f, 3.0f, 0.0f, madron::dsp::ParameterMapping::Enumerated };
    require(std::abs(madron::dsp::normalizedToValue(exponential, 0.5f) - std::sqrt(400000.0f)) < 0.01f, "exponential parameter mapping");
    require(madron::dsp::normalizedToValue(enumerated, 0.51f) == 2.0f, "enumerated parameter mapping");
    madron::inputb::InputBGenerator generator; require(generator.prepare(spec()),"Input B prepare"); auto signal=tone(2048,220.0f);std::vector<float> out(signal.size()),right(signal.size()),outRight(signal.size());for(auto mode:{madron::inputb::Mode::FilteredCopy,madron::inputb::Mode::DistortedCopy,madron::inputb::Mode::DelayedCopy,madron::inputb::Mode::DetunedCopy}){generator.reset();generator.setParameters({mode,1300.0f,0.2f,4.0f,31.0f,100.0f});for(std::size_t offset=0;offset<signal.size();offset+=73){const auto n=std::min<std::size_t>(73,signal.size()-offset);const float* in[]{signal.data()+offset,right.data()+offset};float* o[]{out.data()+offset,outRight.data()+offset};require(generator.process({in,2,n},{o,2,n})==madron::dsp::ProcessResult::Ok,"Input B mode process");}require(finite(out),"Input B finite");}require(rms(out)>0.001f,"Input B creates non-silent transformed signal");
    using namespace madron::random;GraphDescription valid;valid.nodes={{1,NodeType::Input},{2,NodeType::FormantShifter},{3,NodeType::Output}};valid.connections={{1,Port::AudioOut,2,Port::MainIn},{2,Port::AudioOut,3,Port::MainIn}};require(static_cast<bool>(validate(valid)),"valid graph");GraphDescription cycle;cycle.nodes={{1,NodeType::FormantShifter},{2,NodeType::TimbreResynth}};cycle.connections={{1,Port::AudioOut,2,Port::MainIn},{2,Port::AudioOut,1,Port::MainIn}};require(validate(cycle).error==GraphError::Cycle,"cycle rejected");auto invalidBus=valid;invalidBus.connections[0].sourceChannels=3;require(validate(invalidBus).error==GraphError::InvalidAudioBus,"invalid bus rejected");
}
void formantTests() { madron::formant::FormantShifter shifter;require(shifter.prepare(spec()),"Formant prepare");shifter.setParameters({700.0f,10.0f,1.0f});const auto input=tone(12288,220.0f,0.28f);const auto output=renderMonoStereo(shifter,input,nullptr,73);require(finite(output),"Formant finite output");require(rms(output,4096)>0.01f,"Formant has wet energy");double diff=0.0;for(std::size_t i=4096;i<output.size();++i){const float delayed=input[i-shifter.getLatencySamples()];diff+=std::abs(output[i]-delayed);}require(diff>20.0,"Formant shift changes spectral output");shifter.setParameters({0.0f,8.0f,0.0f});shifter.reset();const auto dry=renderMonoStereo(shifter,input,nullptr,61);require(rms(dry,2048)>0.01f,"Formant latency-aligned dry path"); }
void cepstralTests() { madron::cepstral::CepstralMorph morph;require(morph.prepare(spec()),"Cepstral prepare");const auto a=tone(12288,220.0f,0.1f),b=tone(12288,440.0f,0.35f);morph.setParameters({madron::cepstral::FFTSize::N1024,1.0f,1.0f,0.4f,madron::cepstral::PhaseMode::Normal});const auto external=renderMonoStereo(morph,a,&b,61);require(finite(external),"Cepstral external finite");require(rms(external,2048)>0.005f,"Cepstral external energy");morph.reset();morph.setParameters({madron::cepstral::FFTSize::N512,0.8f,0.2f,0.6f,madron::cepstral::PhaseMode::BOnly});const auto internal=renderMonoStereo(morph,a,nullptr,61);require(finite(internal),"Cepstral internal finite");double diff=0.0;for(std::size_t i=2048;i<a.size();++i)diff+=std::abs(external[i]-internal[i]);require(diff>5.0,"Cepstral A/B and internal B differ"); }
void timbreTests() { madron::timbre::PartialTracker tracker;require(tracker.prepare(48000.0),"Partial tracker prepare");const auto input=tone(20000,220.0f);for(float x:input)tracker.push(x);madron::timbre::PartialEvent event{};std::size_t events=0;while(tracker.pop(event))++events;require(events>0,"Partial tracker emits peak events");madron::timbre::TimbreResynth resynth;require(resynth.prepare(spec(127)),"Timbre prepare");resynth.setParameters({1200.0f,300.0f,1.0f,1,0.0f});const auto output=renderMonoStereo(resynth,input,nullptr,127);require(finite(output),"Timbre finite output");require(rms(output,8192)>0.002f,"Timbre grain output has energy");double diff=0.0;for(std::size_t i=8192;i<input.size();++i)diff+=std::abs(output[i]-input[i-resynth.getLatencySamples()]);require(diff>10.0,"Timbre grain resynthesis differs from delayed input"); }
void p03RandomRuntimeTests() {
    using namespace madron::random;
    RecipeGenerator generator;
    std::vector<std::string> topologies;
    for (std::uint64_t seed = 1; seed <= 1000; ++seed) {
        const auto category = static_cast<RecipeCategory>((seed - 1) % 10);
        const auto recipe = generator.generate({{seed}, category, static_cast<std::uint8_t>(seed % 101), static_cast<std::uint8_t>((seed * 7) % 101), PerformanceMode::Balanced, {}});
        require(static_cast<bool>(validate(recipe.graph)), "1000 deterministic recipes validate");
        topologies.push_back(std::to_string(recipe.graph.nodes.size()) + ":" + std::to_string(recipe.graph.connections.size()) + ":" + std::to_string(static_cast<int>(recipe.graph.nodes[1].type)));
        if (seed == 77) { const auto again = generator.generate({{seed}, category, static_cast<std::uint8_t>(seed % 101), static_cast<std::uint8_t>((seed * 7) % 101), PerformanceMode::Balanced, {}}); require(serializeRecipeJson(recipe) == serializeRecipeJson(again), "same seed produces same recipe"); const auto restored = deserializeRecipeJson(serializeRecipeJson(recipe)); require(restored.has_value() && serializeRecipeJson(*restored) == serializeRecipeJson(recipe), "preset JSON round trip"); }
    }
    std::sort(topologies.begin(),topologies.end()); require(std::adjacent_find(topologies.begin(),topologies.end(),std::not_equal_to<>()) != topologies.end(), "random generator produces topology diversity");
    auto recipe = generator.generate({{123456}, RecipeCategory::SpectralFusion, 70, 70, PerformanceMode::Balanced, {}});
    GraphCompiler compiler; ValidationResult error; auto graph = compiler.compile(recipe.graph, {spec(128), 32, 8192}, &error); require(graph != nullptr, "cross-source graph compiles");
    const auto input = tone(16384,220.0f,0.2f); std::vector<float> right(input.size()),out(input.size()),outRight(input.size()); for(std::size_t i=0;i<input.size();++i)right[i]=input[i]*0.7f;
    for(std::size_t offset=0;graph&&offset<input.size();offset+=128){const auto n=std::min<std::size_t>(128,input.size()-offset);const float* in[]{input.data()+offset,right.data()+offset};float* o[]{out.data()+offset,outRight.data()+offset};require(graph->process({in,2,n},{o,2,n})==madron::dsp::ProcessResult::Ok,"compiled graph processes audio");}
    require(finite(out),"compiled graph finite output");
    auto successorRecipe=generator.generate({{654321},RecipeCategory::FormantMutation,55,45,PerformanceMode::Balanced,{}});auto successor=compiler.compile(successorRecipe.graph,{spec(128),32,8192},&error);GraphTransition transition;require(transition.setInitial(std::move(graph))&&transition.request(std::move(successor)),"prepared graphs accept transition");transition.setTransitionSamples(256);for(std::size_t offset=0;offset<1024;offset+=128){const float* in[]{input.data()+offset,right.data()+offset};float* o[]{out.data()+offset,outRight.data()+offset};require(transition.process({in,2,128},{o,2,128})==madron::dsp::ProcessResult::Ok,"crossfade transition processes");}transition.collectRetired();require(finite(out),"crossfade output finite");
    RecipeHistory history;history.push(recipe);auto next=generator.generate({{123457},RecipeCategory::FormantMutation,20,20,PerformanceMode::Eco,{}});history.push(next);require(history.undo()!=nullptr&&history.redo()!=nullptr,"recipe undo redo");
    RecipeLocks locks;locks.structureLocked=true;const auto locked=generator.generate({{999},RecipeCategory::ExtremeProcessing,100,100,PerformanceMode::Extreme,locks},&recipe);require(locked.graph.nodes.size()==recipe.graph.nodes.size()&&locked.graph.connections.size()==recipe.graph.connections.size(),"structure lock preserves prior graph");
}
}
int main(){sharedAndGraphTests();formantTests();cepstralTests();timbreTests();p03RandomRuntimeTests();if(failures==0){std::cout<<"madron P03 DSP and random graph tests passed\n";return 0;}std::cerr<<failures<<" tests failed\n";return 1;}
