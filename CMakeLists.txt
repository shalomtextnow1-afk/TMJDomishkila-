cmake_minimum_required(VERSION 3.22)
project(TMJDomishkilaVoxMagic VERSION 1.0.0)

# Set JUCE_DIR to your JUCE installation, or add JUCE as a subdirectory.
find_package(JUCE CONFIG REQUIRED)

juce_add_plugin(TMJVoxMagic
    COMPANY_NAME "TMJDomishkila"
    BUNDLE_ID "com.tmjdomishkila.voxmagic"
    PLUGIN_MANUFACTURER_CODE TMJD
    PLUGIN_CODE TVM1
    FORMATS VST3 AU
    PRODUCT_NAME "TMJDomishkila Vox Magic"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(TMJVoxMagic PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_features(TMJVoxMagic PUBLIC cxx_std_17)
target_compile_definitions(TMJVoxMagic PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(TMJVoxMagic PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
