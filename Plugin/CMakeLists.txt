juce_add_plugin(Madron
    COMPANY_NAME "NYN Audio"
    PLUGIN_MANUFACTURER_CODE NyNa
    PLUGIN_CODE Madr
    PRODUCT_NAME "Madron"
    VERSION "1.0.0"
    BUNDLE_ID "com.nynaudio.madron"
    FORMATS VST3
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_add_binary_data(MadronAssets
    SOURCES
        ../Resources/fonts/GowunBatang-Regular.ttf)

target_sources(Madron PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(Madron PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(Madron PRIVATE MadronAssets madron::formant madron::cepstral madron::timbre madron::random juce::juce_audio_utils)
