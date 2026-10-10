cmake_minimum_required(VERSION 3.20)

add_library(sub_phatty_core
    src/SynthEngine.cpp
)

target_include_directories(sub_phatty_core PUBLIC
    include
)

target_compile_features(sub_phatty_core PUBLIC cxx_std_20)
