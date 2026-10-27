#pragma once
#include "PluginProcessor.h"
#include <array>

class MadronAudioProcessorEditor final : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit MadronAudioProcessorEditor(MadronAudioProcessor& processor);
    void paint(juce::Graphics&) override; void resized() override;
private:
    class LuckyButton final : public juce::Button { public: LuckyButton():Button("I'm Feeling Lucky"){} void setActive(bool active) noexcept { active_=active; } void setDark(bool dark) noexcept { dark_=dark; } void setDisplayFont(juce::Font font) { font_=std::move(font); } void paintButton(juce::Graphics&,bool,bool) override; private: juce::Font font_{15.0f}; bool active_{},dark_{}; };
    class ThemeButton final : public juce::Button { public: ThemeButton():Button("Theme"){} void setDark(bool dark) noexcept { dark_=dark; } void paintButton(juce::Graphics&,bool,bool) override; private: bool dark_{}; };
    class MixLookAndFeel final : public juce::LookAndFeel_V4 { public: void setDark(bool dark) noexcept { dark_=dark; } void drawLinearSlider(juce::Graphics&,int,int,int,int,float,float,float,const juce::Slider::SliderStyle,juce::Slider&) override; private: bool dark_{}; };
    struct Particle { float x{},y{},vx{},vy{},life{},phase{}; };
    void timerCallback() override; void drawLogo(juce::Graphics&,juce::Rectangle<float>,juce::Colour) const; void resetParticles() noexcept;
    MadronAudioProcessor& processor_; LuckyButton lucky_; ThemeButton theme_; juce::Slider mix_; MixLookAndFeel mixLook_ {}; juce::Font font_ {};
    std::array<Particle,14> particles_ {}; bool dark_ {}, lastActive_ {};
};
