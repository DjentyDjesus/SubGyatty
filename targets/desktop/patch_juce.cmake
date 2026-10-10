set(FILE_TO_PATCH "${JUCE_SRC_DIR}/modules/juce_audio_processors/processors/juce_AudioPluginInstance.h")

if(EXISTS "${FILE_TO_PATCH}")
    file(READ "${FILE_TO_PATCH}" CONTENT)
    string(REGEX REPLACE
        "template <size_t numLayouts>[ \r\n\t]*AudioPluginInstance \\(const short channelLayoutList\\[numLayouts\\]\\[2\\]\\) : AudioProcessor \\(channelLayoutList\\) \\{\\}"
        ""
        CONTENT "${CONTENT}"
    )
    file(WRITE "${FILE_TO_PATCH}" "${CONTENT}")
    message(STATUS "Applied C++20 Clang patch to juce_AudioPluginInstance.h")
else()
    message(WARNING "Could not find ${FILE_TO_PATCH} to patch")
endif()
