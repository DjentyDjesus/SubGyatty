cmake_minimum_required(VERSION 3.20)

add_library(sub_phatty_core
    src/SynthEngine.cpp
)

target_include_directories(sub_phatty_core PUBLIC
    include
)

target_compile_features(sub_phatty_core PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(sub_phatty_core PRIVATE /W4 /O2 /fp:fast /permissive-)
else()
    target_compile_options(sub_phatty_core PRIVATE -Wall -Wextra -O3 -ffast-math)
endif()
