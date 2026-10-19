# pes2013 - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(pes2013 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(PES2013_SOURCES
    src/main.cpp
)

if(WIN32)
    add_executable(pes2013 WIN32 ${PES2013_SOURCES})
else()
    add_executable(pes2013 ${PES2013_SOURCES})
endif()

rexglue_setup_target(pes2013)

# The SDK UI headers include Dear ImGui directly for the desktop host shell.
target_include_directories(pes2013 PRIVATE
    "${REXSDK_DIR}/thirdparty/imgui"
)
