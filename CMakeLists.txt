cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

project(AutoLeaderboard VERSION 1.2.1)

add_library(${PROJECT_NAME} SHARED
    src/main.cpp
    # Add any extra C++ source files here
)

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

# Suppress third-party fmt warning on Clang (unused nodiscard result).
function(suppress_fmt_warning target_name)
    if (NOT TARGET ${target_name})
        return()
    endif()

    get_target_property(fmt_alias ${target_name} ALIASED_TARGET)
    if (fmt_alias)
        set(fmt_target ${fmt_alias})
    else()
        set(fmt_target ${target_name})
    endif()

    get_target_property(fmt_type ${fmt_target} TYPE)
    set(fmt_flag "$<$<CXX_COMPILER_ID:Clang,AppleClang>:-Wno-unused-result>")
    if (fmt_type STREQUAL "INTERFACE_LIBRARY")
        target_compile_options(${fmt_target} INTERFACE ${fmt_flag})
    else()
        target_compile_options(${fmt_target} PRIVATE ${fmt_flag})
    endif()
endfunction()

foreach(fmt_target fmt fmt::fmt fmt-header-only fmt::fmt-header-only)
    suppress_fmt_warning(${fmt_target})
endforeach()

setup_geode_mod(${PROJECT_NAME})
