cmake_minimum_required(VERSION 3.20)
project(SubGyatty_Desktop VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
    PATCH_COMMAND ${CMAKE_COMMAND} -DJUCE_SRC_DIR=<SOURCE_DIR> -P "${CMAKE_CURRENT_LIST_DIR}/patch_juce.cmake"
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(SubGyatty
    COMPANY_NAME "SubGyatty Labs"
    BUNDLE_ID "com.subgyattylabs.subgyatty"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    PLUGIN_MANUFACTURER_CODE SGYA
    PLUGIN_CODE Sgpt
    FORMATS Standalone VST3
    PRODUCT_NAME "SubGyatty"
)

target_sources(SubGyatty PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
)

target_include_directories(SubGyatty PRIVATE
    Source
)

target_link_libraries(SubGyatty PRIVATE
    sub_phatty_core
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_graphics
)

target_compile_definitions(SubGyatty PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_compile_features(SubGyatty PRIVATE cxx_std_20)
