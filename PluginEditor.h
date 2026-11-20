#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TMJVoxMagicAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit TMJVoxMagicAudioProcessorEditor (TMJVoxMagicAudioProcessor&);
    ~TMJVoxMagicAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    TMJVoxMagicAudioProcessor& processor;
    juce::ComboBox mode;
    juce::Slider magic, input, lowCut, comp, presence, air, width, deess, space, output;
    juce::ToggleButton bypass { "Bypass" };
    juce::Label brand, product, modeLabel, magicLabel, advancedLabel;
    std::array<juce::Label, 10> controlLabels;
    std::unique_ptr<ComboAttachment> modeAttachment;
    std::unique_ptr<SliderAttachment> magicAttachment, inputAttachment, lowCutAttachment, compAttachment, presenceAttachment,
        airAttachment, widthAttachment, deessAttachment, spaceAttachment, outputAttachment;
    void styleSlider (juce::Slider&, const juce::String& suffix = {});
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TMJVoxMagicAudioProcessorEditor)
};
