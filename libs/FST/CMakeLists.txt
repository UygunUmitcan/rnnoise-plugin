cmake_minimum_required(VERSION 3.29)

# parse version from fst/fst.h
file(STRINGS fst/fst.h CURRENT_VERSION REGEX "^ *# *define  *FST_MAJOR_VERSION  *([0-9]*)$")
set(major ${CMAKE_MATCH_1})
file(STRINGS fst/fst.h CURRENT_VERSION REGEX "^ *# *define  *FST_MINOR_VERSION  *([0-9]*)$")
set(minor ${CMAKE_MATCH_1})
file(STRINGS fst/fst.h CURRENT_VERSION REGEX "^ *# *define  *FST_MICRO_VERSION  *([0-9]*)$")
set(micro ${CMAKE_MATCH_1})
set(CURRENT_VERSION "${major}.${minor}.${micro}")

project(FST
  VERSION "${CURRENT_VERSION}"
  DESCRIPTION "Free Studio Technology"
  HOMEPAGE_URL https://git.iem.at/zmoelnig/FST
)

message ("-- FST: ${FST_VERSION}")


include_directories(BEFORE ${CMAKE_SOURCE_DIR}/fst)
set(CMAKE_SHARED_MODULE_PREFIX "")

option(FST_DEPRECATE_UNKNOWN "Build with deprecation warnings for unknown FST symbols" OFF)
if(FST_DEPRECATE_UNKNOWN)
else()
  add_compile_definitions(FST_DONT_DEPRECATE_UNKNOWN=1)
endif()


set(FST_BUILD_JUCE AUTO CACHE STRING "Build with JUCE")  # create the variable
set_property(CACHE FST_BUILD_JUCE PROPERTY STRINGS AUTO ON OFF)  #  define list of values GUI will offer for the variable


if(FST_BUILD_JUCE STREQUAL "AUTO")
  find_package(JUCE)
elseif(FST_BUILD_JUCE STREQUAL "ON")
  find_package(JUCE REQUIRED)
endif()

# a simple host application built form scratch
add_executable(FstHost
  src/FstHost/FstHost.cpp
)
target_include_directories(FstHost PRIVATE ${CMAKE_SOURCE_DIR}/src)

# a simple plugin built form scratch
add_library(FstPlugin MODULE
  src/FstPlugin/FstPlugin.cpp
)
target_include_directories(FstPlugin PRIVATE ${CMAKE_SOURCE_DIR}/src)

# a plugin that acts as a proxy for another plugin
# printing out the communication between the host and the proxied plugin
add_library(FstProxy MODULE
  src/FstProxy/FstProxy.cpp
)
target_include_directories(FstProxy PRIVATE ${CMAKE_SOURCE_DIR}/src)

if(${JUCE_FOUND})
  message ("-- FST: Building JUCE plugin & host")

  # JUCE expects the headers to be in pluginterfaces/vst2.x/ ...
  file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/pluginterfaces")
  file(CREATE_LINK "${CMAKE_SOURCE_DIR}/fst" "${CMAKE_BINARY_DIR}/pluginterfaces/vst2.x" SYMBOLIC)
  juce_set_vst2_sdk_path(${CMAKE_BINARY_DIR})


  # a JUCE based plugin
  juce_add_plugin(JstPlugin
    COMPANY_NAME "IEM"
    FORMATS VST
    PRODUCT_NAME "FstJUCEPlugin"
    # VST2 doesn't like versions >0.9 && <1.0, so we just strip the major version
    VERSION "${FST_VERSION_MINOR}.${FST_VERSION_PATCH}"
    PLUGIN_MANUFACTURER_CODE IEMA
    PLUGIN_CODE Jfst
    BUNDLE_ID "at.iem.git.zmoelnig.fst.JstPlugin"
    VST2_CATEGORY kPlugCategEffect
  )
  juce_generate_juce_header(JstPlugin)
  target_sources(JstPlugin PRIVATE
    src/JstPlugin/Source/PluginEditor.cpp
    src/JstPlugin/Source/PluginEditor.h
    src/JstPlugin/Source/PluginProcessor.h
    src/JstPlugin/Source/PluginProcessor.cpp
  )
  target_compile_definitions(JstPlugin PRIVATE
    JUCE_REPORT_APP_USAGE=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    JUCE_VST3_CAN_REPLACE_VST2=0
  )
  target_link_libraries(JstPlugin PRIVATE
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_recommended_warning_flags
  )
  # copy the plugin to the build directory
  get_target_property(jstplugin_bin JstPlugin_VST JUCE_PLUGIN_ARTEFACT_FILE)
  add_custom_command(
    TARGET JstPlugin_VST POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    ${jstplugin_bin}
    ${CMAKE_CURRENT_BINARY_DIR}
  )

  # a JUCE based host
  juce_add_console_app(JstHost
    COMPANY_NAME "IEM"
    FORMATS VST
    PRODUCT_NAME "JstHost"
    BUNDLE_ID "at.iem.git.zmoelnig.fst.JstHost"
  )
  juce_generate_juce_header(JstHost)
  target_sources(JstHost PRIVATE
    src/JstHost/Source/Main.cpp
  )
  target_compile_definitions(JstHost PRIVATE
    JUCE_REPORT_APP_USAGE=0
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP=1
    JUCE_USE_FLAC=0
    JUCE_USE_OGGVORBIS=0
    JUCE_USE_MP3AUDIOFORMAT=0
    JUCE_USE_LAME_AUDIO_FORMAT=0
    JUCE_USE_WINDOWS_MEDIA_FORMAT=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_PLUGINHOST_VST=1
  )
  if(MSVC AND NOT CMAKE_MSVC_RUNTIME_LIBRARY)
    # Default to statically linking the runtime libraries
    set_property(TARGET JstHost PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  endif()

  target_link_libraries(JstHost PRIVATE
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_recommended_warning_flags
  )
  # copy the app to the build directory
  get_target_property(jsthost_dir JstHost RUNTIME_OUTPUT_DIRECTORY)
  add_custom_command(
    TARGET JstHost POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${jsthost_dir}/JstHost"
    ${CMAKE_CURRENT_BINARY_DIR}
  )
endif()


add_subdirectory(tests)
