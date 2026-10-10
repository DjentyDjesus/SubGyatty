cmake_minimum_required(VERSION 3.20)

add_executable(dsp_verification
    dsp_verification.cpp
)

target_link_libraries(dsp_verification PRIVATE
    sub_phatty_core
)

target_include_directories(dsp_verification PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(dsp_verification PRIVATE cxx_std_20)
