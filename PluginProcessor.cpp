#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    float getParam (const TMJVoxMagicAudioProcessor::APVTS& state, const char* id)
    {
        if (auto* p = state.getRawParameterValue (id)) return p->load();
        return 0.0f;
    }
}

TMJVoxMagicAudioProcessor::TMJVoxMagicAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

TMJVoxMagicAudioProcessor::APVTS::ParameterLayout TMJVoxMagicAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    auto choice = [] (const juce::String& id, const juce::String& name, juce::StringArray choices, int def)
    { return std::make_unique<juce::AudioParameterChoice> (id, name, choices, def); };
    auto range = [] (float lo, float hi, float step, float def)
    { return juce::NormalisableRange<float> (lo, hi, step); };
    layout.add (choice ("MODE", "Vocal Style", { "Afrobeat", "Smooth", "Street Power" }, 0));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("MAGIC", "Magic", range (0.0f, 100.0f, 0.1f, 55.0f), 55.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("INPUT", "Input", range (-12.0f, 12.0f, 0.1f, 0.0f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("HPF", "Low Cut", range (40.0f, 140.0f, 1.0f, 75.0f), 75.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("COMP", "Compression", range (0.0f, 100.0f, 0.1f, 55.0f), 55.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("PRES", "Presence", range (-6.0f, 6.0f, 0.1f, 1.5f), 1.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("AIR", "Air", range (-6.0f, 6.0f, 0.1f, 1.5f), 1.5f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("WIDTH", "Width", range (0.0f, 150.0f, 0.1f, 100.0f), 100.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("DEESS", "De-esser", range (0.0f, 100.0f, 0.1f, 45.0f), 45.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("REVERB", "Space", range (0.0f, 30.0f, 0.1f, 0.0f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat> ("OUTPUT", "Output", range (-12.0f, 6.0f, 0.01f, 0.0f), 0.0f));
    layout.add (std::make_unique<juce::AudioParameterBool> ("BYPASS", "Bypass", false));
    return layout;
}

void TMJVoxMagicAudioProcessor::Biquad::set (double nb0, double nb1, double nb2, double na0, double na1, double na2)
{
    b0 = nb0 / na0; b1 = nb1 / na0; b2 = nb2 / na0;
    a1 = na1 / na0; a2 = na2 / na0;
}
float TMJVoxMagicAudioProcessor::Biquad::process (float x)
{
    const double y = b0 * x + z1;
    z1 = b1 * x - a1 * y + z2;
    z2 = b2 * x - a2 * y;
    return (float) y;
}
float TMJVoxMagicAudioProcessor::smooth (float current, float target, float coefficient)
{ return current + coefficient * (target - current); }

void TMJVoxMagicAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (samplesPerBlock);
    currentSampleRate = sampleRate;
    for (auto* filters : { &highPass, &mud, &presence, &air, &deEssDetector })
        for (auto& f : *filters) f.reset();
    compressorEnvelope = { 0.0f, 0.0f };
    deEssEnvelope = { 0.0f, 0.0f };
    limiterGain = 1.0f;
    smoothedInputGain = 1.0f;
    reverb.reset();
    reverbParams.roomSize = 0.25f; reverbParams.damping = 0.65f;
    reverbParams.wetLevel = 0.0f; reverbParams.dryLevel = 1.0f;
    reverbParams.width = 1.0f; reverb.setParameters (reverbParams);
    updateFilters();
}
void TMJVoxMagicAudioProcessor::releaseResources() { reverb.reset(); }
bool TMJVoxMagicAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
}

void TMJVoxMagicAudioProcessor::updateFilters()
{
    const int mode = juce::roundToInt (getParam (parameters, "MODE"));
    const float magic = getParam (parameters, "MAGIC") / 100.0f;
    const float modePresence = mode == 1 ? 0.5f : (mode == 2 ? 2.2f : 1.2f);
    const float modeAir = mode == 1 ? 1.8f : (mode == 2 ? 0.2f : 1.2f);
    const double sr = currentSampleRate;
    const double hpfHz = juce::jlimit (30.0, 180.0, (double) getParam (parameters, "HPF"));
    auto setHP = [&] (Biquad& f) {
        const double w = 2.0 * juce::MathConstants<double>::pi * hpfHz / sr;
        const double c = std::cos (w), s = std::sin (w), alpha = s / (2.0 * 0.707);
        f.set ((1+c)/2, -(1+c), (1+c)/2, 1+alpha, -2*c, 1-alpha);
    };
    auto setPeak = [&] (Biquad& f, double freq, double gainDb, double q) {
        const double A = std::pow (10.0, gainDb / 40.0), w = 2.0 * juce::MathConstants<double>::pi * freq / sr;
        const double c = std::cos(w), alpha = std::sin(w)/(2.0*q);
        f.set (1+alpha*A, -2*c, 1-alpha*A, 1+alpha/A, -2*c, 1-alpha/A);
    };
    auto setShelf = [&] (Biquad& f, double freq, double gainDb) {
        const double A = std::pow (10.0, gainDb/40.0), w=2.0*juce::MathConstants<double>::pi*freq/sr;
        const double c=std::cos(w), s=std::sin(w), alpha=s/2.0*std::sqrt(2.0);
        const double beta=2.0*std::sqrt(A)*alpha;
        f.set (A*((A+1)+(A-1)*c+beta), 2*A*((A-1)+(A+1)*c), A*((A+1)+(A-1)*c-beta),
               (A+1)-(A-1)*c+beta, -2*((A-1)-(A+1)*c), (A+1)-(A-1)*c-beta);
    };
    for (int ch=0; ch<2; ++ch)
    {
        setHP(highPass[(size_t)ch]);
        setPeak(mud[(size_t)ch], 290.0, -1.8 * magic, 0.85);
        setPeak(presence[(size_t)ch], 3200.0, getParam(parameters,"PRES") + modePresence*magic, 0.9);
        setShelf(air[(size_t)ch], 10500.0, getParam(parameters,"AIR") + modeAir*magic);
        // Band-pass detector centred on the sibilance region.
        const double deFreq = juce::jlimit (3500.0, sr * 0.42, 6800.0);
        const double w = 2.0 * juce::MathConstants<double>::pi * deFreq / sr;
        const double c = std::cos (w), alpha = std::sin (w) / (2.0 * 1.15);
        deEssDetector[(size_t)ch].set (alpha, 0.0, -alpha,
                                       1.0 + alpha, -2.0 * c, 1.0 - alpha);
    }
}

void TMJVoxMagicAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin (2, buffer.getNumChannels());
    if (channels == 0) return;

    const bool bypass = getParam (parameters, "BYPASS") > 0.5f;
    if (bypass) return;

    updateFilters();
    const int mode = juce::roundToInt (getParam (parameters, "MODE"));
    const float magic = getParam (parameters, "MAGIC") / 100.0f;
    const float comp = getParam (parameters, "COMP") / 100.0f;
    const float deess = getParam (parameters, "DEESS") / 100.0f;
    const float width = getParam (parameters, "WIDTH") / 100.0f;
    const float targetInputGain = juce::Decibels::decibelsToGain (getParam (parameters, "INPUT"));
    const float outputGain = juce::Decibels::decibelsToGain (getParam (parameters, "OUTPUT"));
    const float threshold = juce::jmap (comp, -10.0f, -25.0f);
    const float ratio = juce::jmap (comp, 1.5f, mode == 2 ? 5.0f : 3.5f);
    const float attack = std::exp (-1.0f / (0.012f * (float) currentSampleRate));
    const float release = std::exp (-1.0f / (0.120f * (float) currentSampleRate));
    const float deEssAttack = std::exp (-1.0f / (0.0015f * (float) currentSampleRate));
    const float deEssRelease = std::exp (-1.0f / (0.075f * (float) currentSampleRate));
    const float inputSmoothing = 1.0f - std::exp (-1.0f / (0.020f * (float) currentSampleRate));
    const float limiterAttack = std::exp (-1.0f / (0.0008f * (float) currentSampleRate));
    const float limiterRelease = std::exp (-1.0f / (0.080f * (float) currentSampleRate));
    const float saturation = 1.0f + magic * (mode == 2 ? 1.15f : (mode == 1 ? 0.28f : 0.55f));
    const float wet = getParam (parameters, "REVERB") / 100.0f;

    reverbParams.roomSize = mode == 1 ? 0.34f : 0.25f;
    reverbParams.damping = mode == 1 ? 0.78f : 0.62f;
    reverbParams.wetLevel = wet * 0.28f;
    reverbParams.dryLevel = 1.0f - wet * 0.10f;
    reverbParams.width = 1.0f;
    reverb.setParameters (reverbParams);

    constexpr float ceiling = 0.89125094f; // -1.0 dBFS sample peak ceiling
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        smoothedInputGain += inputSmoothing * (targetInputGain - smoothedInputGain);
        float samples[2] { buffer.getSample (0, i), channels > 1 ? buffer.getSample (1, i) : buffer.getSample (0, i) };
        float processed[2] { 0.0f, 0.0f };

        for (int ch = 0; ch < channels; ++ch)
        {
            float x = samples[ch] * smoothedInputGain;
            x = highPass[(size_t) ch].process (x);
            x = mud[(size_t) ch].process (x);
            x = presence[(size_t) ch].process (x);
            x = air[(size_t) ch].process (x);

            // Program-dependent compressor with peak envelope and soft-knee transition.
            auto& compEnv = compressorEnvelope[(size_t) ch];
            const float detector = std::abs (x);
            compEnv = detector > compEnv ? attack * compEnv + (1.0f - attack) * detector
                                         : release * compEnv + (1.0f - release) * detector;
            const float levelDb = juce::Decibels::gainToDecibels (juce::jmax (compEnv, 1.0e-7f));
            const float knee = 4.0f;
            const float over = levelDb - threshold;
            const float softOver = over <= -knee * 0.5f ? 0.0f
                : over >= knee * 0.5f ? over
                : std::pow (over + knee * 0.5f, 2.0f) / (2.0f * knee);
            const float compressionAmount = juce::jmax (comp, magic);
            const float reductionDb = softOver * (1.0f - 1.0f / ratio) * compressionAmount;
            x *= juce::Decibels::decibelsToGain (-reductionDb);

            // Split-band de-essing: detect the sibilant band, attenuate only that band.
            const float sib = deEssDetector[(size_t) ch].process (x);
            auto& sibEnv = deEssEnvelope[(size_t) ch];
            const float sibLevel = std::abs (sib);
            sibEnv = sibLevel > sibEnv ? deEssAttack * sibEnv + (1.0f - deEssAttack) * sibLevel
                                       : deEssRelease * sibEnv + (1.0f - deEssRelease) * sibLevel;
            const float sibDb = juce::Decibels::gainToDecibels (juce::jmax (sibEnv, 1.0e-7f));
            const float essReduction = deess * juce::jlimit (0.0f, 0.65f, (sibDb + 30.0f) * 0.018f);
            x -= sib * essReduction;

            // Gentle normalized saturation; mode changes drive rather than output loudness.
            const float driven = std::tanh (x * saturation);
            x = driven / juce::jmax (0.25f, std::tanh (saturation));
            processed[ch] = x;
        }

        if (channels == 2)
        {
            const float mid = 0.5f * (processed[0] + processed[1]);
            const float side = 0.5f * (processed[0] - processed[1]) * width;
            processed[0] = mid + side;
            processed[1] = mid - side;
        }

        for (int ch = 0; ch < channels; ++ch)
            buffer.setSample (ch, i, processed[ch] * outputGain);
    }

    // Reverb is inserted before the final linked peak controller so its tails are protected too.
    if (wet > 0.001f)
    {
        if (channels == 2)
            reverb.processStereo (buffer.getWritePointer (0), buffer.getWritePointer (1), buffer.getNumSamples());
        else
            reverb.processMono (buffer.getWritePointer (0), buffer.getNumSamples());
    }

    // Linked, fast-attack / slow-release output peak control. This is a safety limiter,
    // not a substitute for an oversampled mastering true-peak limiter.
    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float peak = std::abs (buffer.getSample (0, i));
        if (channels > 1) peak = juce::jmax (peak, std::abs (buffer.getSample (1, i)));
        const float wantedGain = peak > ceiling ? ceiling / peak : 1.0f;
        limiterGain = wantedGain < limiterGain
            ? limiterAttack * limiterGain + (1.0f - limiterAttack) * wantedGain
            : limiterRelease * limiterGain + (1.0f - limiterRelease) * wantedGain;
        for (int ch = 0; ch < channels; ++ch)
            buffer.setSample (ch, i, juce::jlimit (-ceiling, ceiling, buffer.getSample (ch, i) * limiterGain));
    }
}

juce::AudioProcessorEditor* TMJVoxMagicAudioProcessor::createEditor() { return new TMJVoxMagicAudioProcessorEditor (*this); }
void TMJVoxMagicAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{ auto state = parameters.copyState(); std::unique_ptr<juce::XmlElement> xml (state.createXml()); copyXmlToBinary(*xml,dest); }
void TMJVoxMagicAudioProcessor::setStateInformation (const void* data, int size)
{ std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary(data,size)); if (xml && xml->hasTagName(parameters.state.getType())) parameters.replaceState(juce::ValueTree::fromXml(*xml)); }
