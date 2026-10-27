#pragma once

#include <atomic>
#include <string_view>

namespace madron::dsp {

enum class ParameterUnit { Generic, Percent, Decibels, Cents, Hertz, Milliseconds, Choice };
enum class ParameterMapping { Linear, Exponential, Enumerated };
enum class SmoothingPolicy { None, LinearRamp, OnePole };

struct ParameterDescriptor {
    std::string_view id;
    std::string_view displayName;
    ParameterUnit unit { ParameterUnit::Generic };
    float minimum {};
    float maximum { 1.0f };
    float defaultValue {};
    ParameterMapping mapping { ParameterMapping::Linear };
    SmoothingPolicy smoothing { SmoothingPolicy::None };
    bool randomizable { true };
    bool lockable { true };
};

inline float normalizedToValue(const ParameterDescriptor& descriptor, const float normalized) noexcept {
    const auto n = normalized < 0.0f ? 0.0f : (normalized > 1.0f ? 1.0f : normalized);
    return descriptor.minimum + (descriptor.maximum - descriptor.minimum) * n;
}

class AtomicParameter {
public:
    explicit AtomicParameter(const ParameterDescriptor& descriptor) noexcept : descriptor_(descriptor), value_(descriptor.defaultValue) {}
    void set(const float value) noexcept { value_.store(value < descriptor_.minimum ? descriptor_.minimum : (value > descriptor_.maximum ? descriptor_.maximum : value), std::memory_order_release); }
    [[nodiscard]] float get() const noexcept { return value_.load(std::memory_order_acquire); }
    [[nodiscard]] const ParameterDescriptor& descriptor() const noexcept { return descriptor_; }
private:
    ParameterDescriptor descriptor_;
    std::atomic<float> value_;
};

} // namespace madron::dsp
