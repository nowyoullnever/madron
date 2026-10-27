#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace madron::random {
using NodeID = std::uint32_t;
enum class NodeType { Input, Output, FormantShifter, CepstralMorph, TimbreResynth, Split, Merge };
enum class Port : std::uint8_t { AudioOut = 0, MainIn = 0, SecondaryIn = 1 };
struct ResourceCost { std::uint16_t units { 1 }; std::uint16_t voices {}; };
struct ParameterValue { std::string id; float value {}; bool locked {}; };
struct ParameterSet { std::vector<ParameterValue> values; };
struct NodeInstance { NodeID id {}; NodeType type {}; ParameterSet parameters {}; float wetDry { 1.0f }; bool locked {}; ResourceCost cost {}; };
struct GraphConnection {
    NodeID source {};
    Port sourcePort { Port::AudioOut };
    NodeID destination {};
    Port destinationPort { Port::MainIn };
    float gain { 1.0f };
    std::uint8_t sourceChannels { 2 };
    std::uint8_t destinationChannels { 2 };
};
struct GraphDescription { std::vector<NodeInstance> nodes; std::vector<GraphConnection> connections; std::uint16_t maximumNodes { 32 }; std::uint16_t maximumCost { 1000 }; };
struct RecipeSeed { std::uint64_t value {}; };
struct RandomConstraint { std::uint16_t maximumNodes { 32 }; std::uint16_t maximumCost { 1000 }; bool allowParallel { true }; };
struct RecipeDefinition { RecipeSeed seed {}; GraphDescription graph; RandomConstraint constraints {}; };
enum class GraphError { None, DuplicateNode, MissingNode, InvalidSourcePort, InvalidDestinationPort, InvalidAudioBus, DuplicateInput, MissingRequiredInput, Cycle, TooManyNodes, CostExceeded };
struct ValidationResult { GraphError error { GraphError::None }; std::string message; [[nodiscard]] explicit operator bool() const noexcept { return error == GraphError::None; } };
ValidationResult validate(const GraphDescription& graph);
} // namespace madron::random
