#include <madron/random/Graph.h>

#include <algorithm>
#include <cmath>
#include <functional>
#include <unordered_map>
#include <queue>
#include <unordered_set>

namespace madron::random {
namespace {
int inputCount(NodeType type) { switch(type) { case NodeType::Input: return 0; case NodeType::CepstralMorph: case NodeType::Merge: return 2; case NodeType::Output: case NodeType::FormantShifter: case NodeType::TimbreResynth: case NodeType::InputBGenerator: case NodeType::Split: return 1; } return 0; }
int outputCount(NodeType type) { return type == NodeType::Output ? 0 : 1; }
bool requiredInput(NodeType type, int port) { return inputCount(type) > 0 && !(type == NodeType::CepstralMorph && port == 1); }
bool parameterValid(const ParameterValue& value) { return !value.id.empty() && std::isfinite(value.value); }
}
ResourceCost estimateCost(const NodeInstance& node) noexcept {
    // Costs are deliberately derived from the actual engine type and selected
    // operating parameters; GraphDescription::cost is metadata, never authority.
    switch (node.type) {
        case NodeType::CepstralMorph: {
            std::uint16_t sizeCost = 160;
            for (const auto& p : node.parameters.values) if (p.id == "cepstral.fft_size") sizeCost = static_cast<std::uint16_t>(80u << static_cast<unsigned>(std::clamp(p.value, 0.0f, 3.0f)));
            return { sizeCost, 0 };
        }
        case NodeType::TimbreResynth: {
            std::uint16_t voices = 20;
            for (const auto& p : node.parameters.values) if (p.id == "timbre.active_voices") voices = static_cast<std::uint16_t>(std::clamp(p.value, 1.0f, 20.0f));
            return { static_cast<std::uint16_t>(35 + voices * 7), voices };
        }
        case NodeType::FormantShifter: return { 90, 0 };
        case NodeType::InputBGenerator: return { 18, 0 };
        case NodeType::Split: case NodeType::Merge: return { 4, 0 };
        case NodeType::Input: case NodeType::Output: return { 1, 0 };
    }
    return { 1000, 0 };
}
std::size_t estimateMemoryBytes(const NodeInstance& node) noexcept {
    switch(node.type) {
        case NodeType::CepstralMorph: { std::size_t fft=1024; for(const auto& p:node.parameters.values)if(p.id=="cepstral.fft_size")fft=512u<<static_cast<unsigned>(std::clamp(p.value,0.0f,3.0f)); return fft*2u*14u*sizeof(float); }
        case NodeType::TimbreResynth: return 2u*(16384u+32768u+32768u)*sizeof(float);
        case NodeType::FormantShifter: return 2u*4096u*8u*sizeof(float);
        case NodeType::InputBGenerator: return 2u*24000u*sizeof(float);
        case NodeType::Split: case NodeType::Merge: return 2u*1024u*sizeof(float);
        default: return 0;
    }
}
ValidationResult validate(const GraphDescription& graph) {
    if (graph.nodes.size() > graph.maximumNodes) return {GraphError::TooManyNodes, "node limit exceeded"};
    std::unordered_map<NodeID, NodeType> types; std::uint32_t cost=0; std::size_t memory=0, inputs=0, outputs=0;
    for (const auto& node: graph.nodes) { if (!types.emplace(node.id,node.type).second) return {GraphError::DuplicateNode,"duplicate node id"}; if(node.type==NodeType::Input)++inputs; if(node.type==NodeType::Output)++outputs; for(const auto& p:node.parameters.values) if(!parameterValid(p)) return {GraphError::InvalidParameter,"invalid parameter"}; cost += estimateCost(node).units; memory+=estimateMemoryBytes(node); }
    if (cost > graph.maximumCost) return {GraphError::CostExceeded,"resource cost exceeded"}; if(memory>graph.maximumMemoryBytes)return {GraphError::CostExceeded,"memory budget exceeded"};
    std::unordered_set<std::uint64_t> occupied; std::unordered_map<NodeID,std::vector<NodeID>> edges;
    for (const auto& edge: graph.connections) {
        const auto source=types.find(edge.source), destination=types.find(edge.destination);
        if(source==types.end() || destination==types.end()) return {GraphError::MissingNode,"connection references missing node"};
        if((edge.sourceChannels!=1 && edge.sourceChannels!=2) || (edge.destinationChannels!=1 && edge.destinationChannels!=2) || edge.sourceChannels!=edge.destinationChannels) return {GraphError::InvalidAudioBus,"unsupported or mismatched audio bus"};
        if(static_cast<int>(edge.sourcePort)>=outputCount(source->second)) return {GraphError::InvalidSourcePort,"invalid source port"};
        if(static_cast<int>(edge.destinationPort)>=inputCount(destination->second)) return {GraphError::InvalidDestinationPort,"invalid destination port"};
        const std::uint64_t key=(static_cast<std::uint64_t>(edge.destination)<<8u)|static_cast<std::uint8_t>(edge.destinationPort);
        if(!occupied.insert(key).second) return {GraphError::DuplicateInput,"input port already connected"};
        edges[edge.source].push_back(edge.destination);
    }
    for(const auto& [id,type]:types) for(int port=0;port<inputCount(type);++port) if(requiredInput(type,port)) { const auto key=(static_cast<std::uint64_t>(id)<<8u)|static_cast<std::uint8_t>(port); if(!occupied.contains(key)) return {GraphError::MissingRequiredInput,"required input is unconnected"}; }
    std::unordered_set<NodeID> visiting, visited;
    std::function<bool(NodeID)> visit=[&](NodeID id){ if(visiting.contains(id)) return true; if(visited.contains(id)) return false; visiting.insert(id); for(NodeID next:edges[id]) if(visit(next)) return true; visiting.erase(id); visited.insert(id); return false; };
    for(const auto& [id,_]:types) if(visit(id)) return {GraphError::Cycle,"cycle detected"};
    if(inputs != 1) return {GraphError::MissingInputNode,"graph requires exactly one input node"};
    if(outputs != 1) return {GraphError::MissingOutputNode,"graph requires exactly one output node"};
    NodeID inputId{}; for(const auto& node:graph.nodes) if(node.type==NodeType::Input) inputId=node.id;
    std::unordered_set<NodeID> reachable; std::queue<NodeID> pending; pending.push(inputId); reachable.insert(inputId);
    while(!pending.empty()){const auto current=pending.front();pending.pop();for(const auto next:edges[current])if(reachable.insert(next).second)pending.push(next);}
    for(const auto& node:graph.nodes) if(!reachable.contains(node.id)) return {GraphError::UnreachableNode,"node is not reachable from input"};
    return {};
}
} // namespace madron::random
