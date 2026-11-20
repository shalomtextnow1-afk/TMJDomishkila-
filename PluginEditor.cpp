#include "PluginEditor.h"

namespace
{
    const juce::Colour black { 0xff101010 };
    const juce::Colour panel { 0xff1b1b1b };
    const juce::Colour gold { 0xffd6ad55 };
    const juce::Colour mutedGold { 0xff8f7138 };
}

TMJVoxMagicAudioProcessorEditor::TMJVoxMagicAudioProcessorEditor (TMJVoxMagicAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (760, 590);
    brand.setText ("TMJDOMISHKILA", juce::dontSendNotification);
    brand.setFont (juce::Font (14.0f, juce::Font::bold));
    brand.setColour (juce::Label::textColourId, gold);
    brand.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (brand);
    product.setText ("VOX MAGIC", juce::dontSendNotification);
    product.setFont (juce::Font (30.0f, juce::Font::bold));
    product.setColour (juce::Label::textColourId, juce::Colours::white);
    product.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (product);

    mode.addItemList ({ "Afrobeat", "Smooth", "Street Power" }, 1);
    mode.setColour (juce::ComboBox::backgroundColourId, panel);
    mode.setColour (juce::ComboBox::textColourId, gold);
    mode.setColour (juce::ComboBox::outlineColourId, mutedGold);
    addAndMakeVisible (mode);
    modeAttachment = std::make_unique<ComboAttachment> (processor.parameters, "MODE", mode);

    styleSlider (magic, "%"); styleSlider (input, " dB"); styleSlider (lowCut, " Hz"); styleSlider (comp, "%"); styleSlider (presence, " dB");
    styleSlider (air, " dB"); styleSlider (width, "%"); styleSlider (deess, "%");
    styleSlider (space, "%"); styleSlider (output, " dB");
    addAndMakeVisible (magic); addAndMakeVisible (input); addAndMakeVisible (lowCut); addAndMakeVisible (comp); addAndMakeVisible (presence);
    addAndMakeVisible (air); addAndMakeVisible (width); addAndMakeVisible (deess);
    addAndMakeVisible (space); addAndMakeVisible (output); addAndMakeVisible (bypass);
    magicAttachment = std::make_unique<SliderAttachment> (processor.parameters,"MAGIC",magic);
    inputAttachment = std::make_unique<SliderAttachment> (processor.parameters,"INPUT",input);
    lowCutAttachment = std::make_unique<SliderAttachment> (processor.parameters,"HPF",lowCut);
    compAttachment = std::make_unique<SliderAttachment> (processor.parameters,"COMP",comp);
    presenceAttachment = std::make_unique<SliderAttachment> (processor.parameters,"PRES",presence);
    airAttachment = std::make_unique<SliderAttachment> (processor.parameters,"AIR",air);
    widthAttachment = std::make_unique<SliderAttachment> (processor.parameters,"WIDTH",width);
    deessAttachment = std::make_unique<SliderAttachment> (processor.parameters,"DEESS",deess);
    spaceAttachment = std::make_unique<SliderAttachment> (processor.parameters,"REVERB",space);
    outputAttachment = std::make_unique<SliderAttachment> (processor.parameters,"OUTPUT",output);
    bypass.setColour (juce::ToggleButton::textColourId, gold);
    addAndMakeVisible (bypass);
    bypass.onClick = [this] { processor.parameters.getParameter("BYPASS")->setValueNotifyingHost(bypass.getToggleState() ? 1.0f : 0.0f); };
    bypass.setToggleState (processor.parameters.getRawParameterValue("BYPASS")->load() > 0.5f, juce::dontSendNotification);

    const juce::String names[10] { "MAGIC", "INPUT GAIN", "LOW CUT", "COMPRESSION", "PRESENCE", "AIR", "WIDTH", "DE-ESSER", "SPACE", "OUTPUT" };
    for (size_t i = 0; i < controlLabels.size(); ++i)
    {
        controlLabels[i].setText (names[i], juce::dontSendNotification);
        controlLabels[i].setFont (juce::Font (10.5f, juce::Font::bold));
        controlLabels[i].setColour (juce::Label::textColourId, gold);
        controlLabels[i].setJustificationType (juce::Justification::centred);
        addAndMakeVisible (controlLabels[i]);
    }

    advancedLabel.setText ("SHAPE YOUR SOUND", juce::dontSendNotification);
    advancedLabel.setFont (juce::Font (13.0f, juce::Font::bold));
    advancedLabel.setColour (juce::Label::textColourId, gold);
    addAndMakeVisible (advancedLabel);
}

void TMJVoxMagicAudioProcessorEditor::styleSlider (juce::Slider& s, const juce::String& suffix)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    s.setTextValueSuffix (suffix);
    s.setColour (juce::Slider::rotarySliderFillColourId, gold);
    s.setColour (juce::Slider::rotarySliderOutlineColourId, mutedGold);
    s.setColour (juce::Slider::thumbColourId, juce::Colours::white);
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    s.setColour (juce::Slider::textBoxOutlineColourId, mutedGold);
    s.setColour (juce::Slider::textBoxBackgroundColourId, panel);
}

void TMJVoxMagicAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (black);
    g.setColour (gold.withAlpha (0.8f));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (8.0f), 14.0f, 1.4f);
    g.setColour (panel);
    g.fillRoundedRectangle (juce::Rectangle<float> (22, 92, 716, 156), 12.0f);
    g.fillRoundedRectangle (juce::Rectangle<float> (22, 278, 716, 278), 12.0f);
    g.setColour (mutedGold);
    g.drawLine (30.0f, 262.0f, 730.0f, 262.0f, 1.0f);
}

void TMJVoxMagicAudioProcessorEditor::resized()
{
    brand.setBounds (20, 12, 720, 20);
    product.setBounds (20, 32, 720, 42);
    mode.setBounds (275, 76, 210, 28);
    controlLabels[0].setBounds (284, 106, 192, 18);
    magic.setBounds (284, 118, 192, 126);
    advancedLabel.setBounds (30, 267, 300, 22);
    const int x[5] { 35, 180, 325, 470, 615 };
    juce::Slider* sliders[5] { &input, &lowCut, &comp, &presence, &air };
    for (int i = 0; i < 5; ++i)
    {
        controlLabels[(size_t) i + 1].setBounds (x[i], 289, 125, 18);
        sliders[i]->setBounds (x[i], 303, 125, 88);
    }
    juce::Slider* secondRow[4] { &width, &deess, &space, &output };
    const int secondLabel[4] { 6, 7, 8, 9 };
    const int secondX[4] { 105, 255, 405, 555 };
    for (int i = 0; i < 4; ++i)
    {
        controlLabels[(size_t) secondLabel[i]].setBounds (secondX[i], 397, 125, 18);
        secondRow[i]->setBounds (secondX[i], 411, 125, 88);
    }
    bypass.setBounds (655, 16, 80, 24);
}
