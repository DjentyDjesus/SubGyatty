#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

namespace sub_phatty::test {

class WavWriter {
public:
    static bool writeWav16(const std::string& filename,
                           const float* samplesLeft,
                           const float* samplesRight,
                           size_t numSamples,
                           uint32_t sampleRate) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;

        const uint16_t numChannels = (samplesRight != nullptr) ? 2 : 1;
        const uint16_t bitsPerSample = 16;
        const uint16_t bytesPerSample = bitsPerSample / 8;
        const uint16_t blockAlign = numChannels * bytesPerSample;
        const uint32_t byteRate = sampleRate * blockAlign;
        const uint32_t dataBytes = static_cast<uint32_t>(numSamples * blockAlign);
        const uint32_t totalChunkSize = 36 + dataBytes;

        // RIFF header
        file.write("RIFF", 4);
        write32LE(file, totalChunkSize);
        file.write("WAVE", 4);

        // fmt sub-chunk
        file.write("fmt ", 4);
        write32LE(file, 16); // Subchunk1Size for PCM
        write16LE(file, 1);  // AudioFormat 1 = PCM
        write16LE(file, numChannels);
        write32LE(file, sampleRate);
        write32LE(file, byteRate);
        write16LE(file, blockAlign);
        write16LE(file, bitsPerSample);

        // data sub-chunk
        file.write("data", 4);
        write32LE(file, dataBytes);

        // Write PCM samples
        for (size_t i = 0; i < numSamples; ++i) {
            float sL = samplesLeft[i];
            sL = std::clamp(sL, -1.0f, 1.0f);
            int16_t pcmL = static_cast<int16_t>(sL * 32767.0f);
            write16LE(file, static_cast<uint16_t>(pcmL));

            if (numChannels == 2) {
                float sR = samplesRight[i];
                sR = std::clamp(sR, -1.0f, 1.0f);
                int16_t pcmR = static_cast<int16_t>(sR * 32767.0f);
                write16LE(file, static_cast<uint16_t>(pcmR));
            }
        }

        return true;
    }

private:
    static void write16LE(std::ofstream& f, uint16_t val) {
        uint8_t bytes[2] = {
            static_cast<uint8_t>(val & 0xFF),
            static_cast<uint8_t>((val >> 8) & 0xFF)
        };
        f.write(reinterpret_cast<const char*>(bytes), 2);
    }

    static void write32LE(std::ofstream& f, uint32_t val) {
        uint8_t bytes[4] = {
            static_cast<uint8_t>(val & 0xFF),
            static_cast<uint8_t>((val >> 8) & 0xFF),
            static_cast<uint8_t>((val >> 16) & 0xFF),
            static_cast<uint8_t>((val >> 24) & 0xFF)
        };
        f.write(reinterpret_cast<const char*>(bytes), 4);
    }
};

} // namespace sub_phatty::test
