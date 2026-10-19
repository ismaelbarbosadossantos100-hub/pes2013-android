cmake_minimum_required(VERSION 3.25)
project(pes2013_android LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# PES2013_ROOT can come from gradle arguments or environment
if(NOT DEFINED PES2013_ROOT)
    if(DEFINED ENV{PES2013_ROOT})
        set(PES2013_ROOT "$ENV{PES2013_ROOT}")
    else()
        message(FATAL_ERROR "PES2013_ROOT must be set (gradle arg -DPES2013_ROOT=... or env var)")
    endif()
endif()

if(NOT DEFINED REXSDK_DIR)
    if(DEFINED ENV{REXSDK_DIR})
        set(REXSDK_DIR "$ENV{REXSDK_DIR}")
    else()
        message(FATAL_ERROR "REXSDK_DIR must be set (gradle arg -DREXSDK_DIR=... or env var)")
    endif()
endif()

message(STATUS "PES2013_ROOT = ${PES2013_ROOT}")
message(STATUS "REXSDK_DIR  = ${REXSDK_DIR}")

set(REXGLUE_HOST_TARGET main)
include("${PES2013_ROOT}/generated/rexglue.cmake")

add_library(main SHARED
    "${PES2013_ROOT}/src/main.cpp"
)

# SDLActivity loads libmain.so and invokes the SDL entry point.
rexglue_setup_target(main)

target_include_directories(main PRIVATE
    "${PES2013_ROOT}"
    "${PES2013_ROOT}/src"
    "${PES2013_ROOT}/generated/default"
    "${REXSDK_DIR}/thirdparty/imgui"
)

target_compile_definitions(main PRIVATE
    REX_ANDROID_TOUCH_OVERLAY=1
    REX_ANDROID_MALI_G57=1
    REXGLUE_ENABLE_INDIRECT_DIAGNOSTIC=1
)

set_target_properties(main PROPERTIES
    OUTPUT_NAME "main"
)
