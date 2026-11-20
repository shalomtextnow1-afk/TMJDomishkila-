#pragma once
#include <JuceHeader.h>

class TMJVoxMagicAudioProcessor final : public juce::AudioProcessor
{
public:
    TMJVoxMagicAudioProcessor();
    ~TMJVoxMagicAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "TMJDomishkila Vox Magic"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;

    static APVTS::ParameterLayout createParameterLayout();

private:
    struct Biquad
    {
        double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
        double z1 = 0.0, z2 = 0.0;
        void set (double nb0, double nb1, double nb2, double na0, double na1, double na2);
        float process (float x);
        void reset() { z1 = z2 = 0.0; }
    };

    void updateFilters();
    static float smooth (float current, float target, float coefficient);

    double currentSampleRate = 44100.0;
    std::array<Biquad, 2> highPass, mud, presence, air, deEssDetector;
    std::array<float, 2> compressorEnvelope { 0.0f, 0.0f };
    std::array<float, 2> deEssEnvelope { 0.0f, 0.0f };
    float limiterGain = 1.0f;
    float smoothedInputGain = 1.0f;
    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TMJVoxMagicAudioProcessor)
};
