# TMJDomishkila Vox Magic

A JUCE-based macOS vocal-effect plugin project for **VST3** and **Audio Unit (AU)**. The interface uses a black-and-gold identity and offers Afrobeat, Smooth and Street Power voicings, a main Magic macro, and direct controls for shaping the vocal chain.

## Current source features

- Three selectable voicings: Afrobeat, Smooth and Street Power
- Magic macro for coordinated tone, compression and saturation changes
- Input trim with smoothing and adjustable 40–140 Hz low-cut filter
- Low-mid cleanup, presence bell and high-shelf air EQ
- Program-dependent compressor with attack/release envelope, soft-knee transfer curve and independent advanced compression amount
- Split-band sibilance detector centered around 6.8 kHz; attenuation is applied to the detected band rather than the whole vocal
- Mode-dependent saturation
- Mid/side stereo-width control
- Reverb for mono and stereo buses
- Linked output peak controller targeting -1 dBFS sample peaks
- Host-automatable parameters and DAW state recall
- Custom black-and-gold editor

## Engineering notes and limitations

This is still a development build, not a finished commercial vocal suite. The Magic macro is deterministic processing, not machine-learning source analysis. It cannot guarantee a radio-ready result from every microphone, room, performance or backing track. The output controller is a sample-peak safety stage, **not** an oversampled true-peak mastering limiter. The compressor is a straightforward feed-forward design and should be auditioned carefully on real vocals. The current reverb is JUCE's built-in reverb. No pitch correction, noise removal, automatic gain staging, lookahead limiting, oversampling or factory-preset browser is implemented yet.

The DSP and UI changes in this iteration have not been compiled or auditioned because JUCE is not installed in the available development environment. Do not treat this source update as a tested binary.

## Build on macOS

Requirements:
- macOS and Xcode / Xcode Command Line Tools
- CMake 3.22 or newer
- JUCE 7 or JUCE 8 with its CMake package configuration

If JUCE is installed at `~/JUCE`:

```bash
cmake -S . -B build -DJUCE_DIR="$HOME/JUCE/lib/cmake/JUCE" -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The configured targets are `TMJVoxMagic_VST3` and `TMJVoxMagic_AU`. Exact output paths depend on the generator and JUCE installation. Copy the resulting `.vst3` to `~/Library/Audio/Plug-Ins/VST3/` and the `.component` to `~/Library/Audio/Plug-Ins/Components/`, then rescan plug-ins in Cubase 15.

## Changelog

### Development iteration 0.2
- Replaced the broadband level-triggered de-esser approximation with a dedicated sibilance-band detector and split-band attenuation.
- Added adjustable input trim and low-cut controls to the plug-in and editor.
- Added a soft-knee compressor curve and made the compressor ratio vary by vocal mode.
- Moved reverb ahead of final peak protection and added mono reverb processing.
- Replaced the simple tanh output stage with a linked fast-attack / slow-release sample-peak controller targeting -1 dBFS.
- Fixed vocal-mode decoding so all three choice values map correctly from the host parameter representation.
- Expanded the editor layout to fit the added controls.

## Release checklist

1. Compile on macOS with a supported JUCE version and resolve any compiler/API differences.
2. Test VST3 and AU loading in Cubase 15 and at least one AU host.
3. Verify mono/stereo, 44.1/48/88.2/96 kHz, block-size changes, automation, bypass and state recall.
4. Measure latency, CPU use, denormal behavior and peak ceiling; add oversampling/lookahead if a true-peak ceiling is required.
5. Audition on multiple male/female vocals, sibilant material, plosives, quiet takes and untreated-room recordings.
6. Add factory presets, accessibility refinements, code signing and notarization before distribution.
