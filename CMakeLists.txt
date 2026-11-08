cmake_minimum_required(VERSION 3.22)
project(MayStudioMidiDrummer VERSION 1.4.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "")

# JUCE: usa una copia locale (-DJUCE_DIR=...) oppure la scarica automaticamente
if(DEFINED JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
endif()

set(MAY_FORMATS VST3 Standalone)
if(APPLE)
    list(APPEND MAY_FORMATS AU)
endif()

juce_add_plugin(MayStudioMidiDrummer
    COMPANY_NAME "May Studio Recording"
    COMPANY_WEBSITE "https://vincemay.it"
    PRODUCT_NAME "May Studio Midi Drummer"
    BUNDLE_ID com.maystudio.mididrummer
    PLUGIN_MANUFACTURER_CODE MaSt
    PLUGIN_CODE MsMd
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Drum Generator
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
    FORMATS ${MAY_FORMATS})

juce_add_binary_data(MayDrummerData SOURCES Resources/logo.jpg)

target_sources(MayStudioMidiDrummer PRIVATE
    Source/PatternLibrary.cpp
    Source/DrumGenerator.cpp
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(MayStudioMidiDrummer PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

if(MSVC)
    target_compile_options(MayStudioMidiDrummer PRIVATE /utf-8)
endif()

target_link_libraries(MayStudioMidiDrummer
    PRIVATE
        MayDrummerData
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
