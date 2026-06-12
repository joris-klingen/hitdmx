cmake_minimum_required(VERSION 3.22)

project(hitccdmx VERSION 0.2.0 LANGUAGES C CXX)

if(NOT APPLE)
    message(FATAL_ERROR "hitccdmx targets macOS only. Build on a Mac.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(hitccdmx
    COMPANY_NAME                "hitdmx"
    BUNDLE_ID                   "com.hitdmx.hitccdmx"
    PLUGIN_MANUFACTURER_CODE    "HtDx"
    PLUGIN_CODE                 "Hccd"
    FORMATS                     VST3
    PRODUCT_NAME                "hitccdmx"
    DESCRIPTION                 "Exposes DMX-512 channels as plain DAW-automatable (CC-style) parameters"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    VST3_CATEGORIES             "Fx" "Tools"
)

target_sources(hitccdmx PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GuiParams.cpp
    Source/EnttecProDmx.cpp
)

target_include_directories(hitccdmx PRIVATE
    Source
)

target_compile_definitions(hitccdmx PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(hitccdmx PRIVATE
    "-framework IOKit"
    "-framework CoreFoundation"
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
