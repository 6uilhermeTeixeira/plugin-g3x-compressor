cmake_minimum_required(VERSION 3.22)

project(G3XRComp VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(G3X_BUILD_TESTS "Build DSP unit tests" ON)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(G3XRComp
    COMPANY_NAME "G3X Audio"
    BUNDLE_ID "audio.g3x.rcomp"
    PLUGIN_MANUFACTURER_CODE G3Xa
    PLUGIN_CODE G3rc
    FORMATS Standalone VST3
    PRODUCT_NAME "G3X RComp"
    VST3_CATEGORIES Fx Dynamics
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(G3XRComp PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Dsp/CompressorEngine.cpp
    Source/Dsp/CompressorEngine.h
    Source/UI/LookAndFeel.cpp
    Source/UI/LookAndFeel.h
    Source/UI/Meter.cpp
    Source/UI/Meter.h
)

target_compile_definitions(G3XRComp PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(G3XRComp PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if(G3X_BUILD_TESTS)
    enable_testing()
    add_executable(G3XRCompTests
        Tests/CompressorEngineTests.cpp
        Source/Dsp/CompressorEngine.cpp
    )
    target_include_directories(G3XRCompTests PRIVATE Source)
    target_compile_definitions(G3XRCompTests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
    target_link_libraries(G3XRCompTests PRIVATE
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
    )
    add_test(NAME dsp-tests COMMAND G3XRCompTests)
endif()
