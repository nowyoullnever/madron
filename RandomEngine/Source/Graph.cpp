#include <madron/random/Graph.h>

#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace madron::random {
namespace {
int inputCount(NodeType type) { switch(type) { case NodeType::Input: return 0; case NodeType::CepstralMorph: return 2; case NodeType::Merge: return 2; case NodeType::Output: case NodeType::FormantShifter: case NodeType::TimbreResynth: case NodeType::Split: return 1; } return 0; }
int outputCount(NodeType type) { return type == NodeType::Output ? 0 : 1; }
bool requiredInput(NodeType type, int port) { return inputCount(type) > 0 && !(type == NodeType::CepstralMorph && port == 1); }
}
ValidationResult validate(const GraphDescription& graph) {
    if (graph.nodes.size() > graph.maximumNodes) return {GraphError::TooManyNodes, "node limit exceeded"};
    std::unordered_map<NodeID, NodeType> types; std::uint32_t cost=0;
    for (const auto& node: graph.nodes) { if (!types.emplace(node.id,node.type).second) return {GraphError::DuplicateNode,"duplicate node id"}; cost += node.cost.units; }
    if (cost > graph.maximumCost) return {GraphError::CostExceeded,"resource cost exceeded"};
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
    return {};
}
} // namespace madron::random
