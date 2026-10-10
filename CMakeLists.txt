cmake_minimum_required(VERSION 3.20)
project(SubGyatty VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Core DSP library (Zero external dependencies)
add_subdirectory(core)

# Offline DSP verification and test suite
add_subdirectory(tests)

# Desktop Standalone & VST3 Plugin Target (JUCE)
add_subdirectory(targets/desktop)
