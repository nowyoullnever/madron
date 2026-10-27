#pragma once

#include <madron/random/Graph.h>

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace madron::random {
enum class RecipeCategory : std::uint8_t { FormantMutation, SpectralFusion, HarmonicDestruction, PitchDeconstruction, GranularReconstruction, TextureMutation, SpectralSmearing, UnstableTransformation, ExtremeProcessing, SubtleTransformation };
enum class PerformanceMode : std::uint8_t { Eco, Balanced, Extreme };
struct RecipeLocks { bool structureLocked {}; std::vector<ParameterValue> parameters {}; };
struct Recipe {
    static constexpr std::uint32_t schemaVersion = 1, generatorVersion = 1;
    RecipeSeed seed {}; RecipeCategory category { RecipeCategory::TextureMutation }; std::uint8_t intensity { 50 }, complexity { 50 }; PerformanceMode performance { PerformanceMode::Balanced };
    GraphDescription graph {}; RecipeLocks locks {}; bool fallbackUsed {};
};
struct RecipeRequest { RecipeSeed seed {}; RecipeCategory category { RecipeCategory::TextureMutation }; std::uint8_t intensity { 50 }, complexity { 50 }; PerformanceMode performance { PerformanceMode::Balanced }; RecipeLocks locks {}; };

class RecipeGenerator final {
public:
    [[nodiscard]] Recipe generate(const RecipeRequest& request, const Recipe* previous = nullptr) const;
    [[nodiscard]] static const char* categoryName(RecipeCategory category) noexcept;
    [[nodiscard]] static std::optional<RecipeCategory> parseCategory(const std::string& name) noexcept;
};

class RecipeHistory final {
public:
    explicit RecipeHistory(std::size_t capacity = 32) : capacity_(capacity) {}
    void push(const Recipe& recipe); [[nodiscard]] const Recipe* undo() noexcept; [[nodiscard]] const Recipe* redo() noexcept; [[nodiscard]] const Recipe* current() const noexcept;
    void setFavorite(const Recipe& recipe) { favorite_ = recipe; } [[nodiscard]] const std::optional<Recipe>& favorite() const noexcept { return favorite_; }
private: std::size_t capacity_; std::vector<Recipe> entries_ {}; std::size_t cursor_ {}; std::optional<Recipe> favorite_ {};
};

[[nodiscard]] std::string serializeRecipeJson(const Recipe& recipe);
[[nodiscard]] std::optional<Recipe> deserializeRecipeJson(const std::string& json, std::string* error = nullptr);
} // namespace madron::random
