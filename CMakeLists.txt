cmake_minimum_required(VERSION 3.22)

project(HitDmx VERSION 0.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HITDMX_USE_FTDI_D2XX "Build with ENTTEC DMX USB Pro support via FTDI D2XX" OFF)
set(HITDMX_FTDI_D2XX_DIR "" CACHE PATH "Path to FTDI D2XX SDK (containing ftd2xx.h and import lib)")

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(HitDmx
    COMPANY_NAME            "HitDmx"
    BUNDLE_ID               "com.hitdmx.HitDmx"
    PLUGIN_MANUFACTURER_CODE "HtDx"
    PLUGIN_CODE             "Hdmx"
    FORMATS                 VST3 Standalone
    PRODUCT_NAME            "HitDmx"
    DESCRIPTION             "DMX-512 lighting controller as an audio plugin"
    IS_SYNTH                FALSE
    NEEDS_MIDI_INPUT        FALSE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES         "Fx" "Tools"
)

target_sources(HitDmx PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/GuiParams.cpp
    Source/Dmx/DmxBackend.cpp
    Source/Dmx/NullDmxBackend.cpp
    Source/Dmx/EnttecProBackend.cpp
)

target_include_directories(HitDmx PRIVATE Source)

target_compile_definitions(HitDmx PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(HitDmx PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if(HITDMX_USE_FTDI_D2XX)
    if(NOT HITDMX_FTDI_D2XX_DIR)
        message(FATAL_ERROR "HITDMX_USE_FTDI_D2XX is ON but HITDMX_FTDI_D2XX_DIR is not set.")
    endif()
    target_compile_definitions(HitDmx PRIVATE HITDMX_HAVE_FTDI_D2XX=1)
    target_include_directories(HitDmx PRIVATE "${HITDMX_FTDI_D2XX_DIR}")
    if(WIN32)
        target_link_libraries(HitDmx PRIVATE "${HITDMX_FTDI_D2XX_DIR}/ftd2xx.lib")
    else()
        target_link_libraries(HitDmx PRIVATE ftd2xx)
    endif()
endif()
