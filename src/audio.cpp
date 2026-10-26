#include "audio.hpp"
#include "console.hpp"
#include "nes.hpp"
#include "nes/nes_apu.hpp"
#include "main.hpp"

#define CYCLES_PER_SAMPLE_NTSC (1789773.0 / 44100.0)
#define CYCLES_PER_SAMPLE_PAL (1662607.0 / 44100.0)
#define CYCLES_PER_SAMPLE_GB  (4194304.0 / 44100.0)

Audio audioSystem;

void SDLCALL audioCallback(void *userdata, SDL_AudioStream *stream, int additional, UNUSED int total) {
    Audio* audio = (Audio*)(userdata);

    if (additional <= 0) {
        return;
    }

    int samplesRequested = additional / sizeof(int16_t);
    std::vector<int16_t> out(samplesRequested);

    std::lock_guard<std::mutex> lock(audio->mutex);

    for (int i = 0; i < samplesRequested; i++) {
        double s = 0.0;
        if (!audio->buffer.empty()) {
            s = audio->buffer.front();
            audio->buffer.pop();
        }

        out[i] = (int16_t)(s * 30000.0);
    }

    SDL_PutAudioStreamData(stream, out.data(), samplesRequested * sizeof(int16_t));
}

void Audio::init() {
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        DebugPrintLog("AUDIO", "Failed to initialize SDL audio");
        return;
    }

    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.freq = sampleRate;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;

    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audioCallback, this);
    if (!stream) {
        DebugPrintLog("AUDIO", "Failed to open audio device stream");
        return;
    }

    SDL_ResumeAudioStreamDevice(stream);
    DebugPrintLog("AUDIO", "Initialized SDL Audio");
}


void Audio::close() {
    if (stream) {
        SDL_DestroyAudioStream(stream);
        stream = nullptr;
    }
    DebugPrintLog("AUDIO", "Shutdown SDL Audio");
}

void Audio::advance(uint32_t cycles) {
    double cyclesPerSample = CYCLES_PER_SAMPLE_NTSC;
    if (emuConsole->getConsoleType() == ConsoleType::GAMEBOY) {
        cyclesPerSample = CYCLES_PER_SAMPLE_GB;
    } else {
        if (getRom() && getRom()->Region == ConsoleRegion::PAL) {
            cyclesPerSample = CYCLES_PER_SAMPLE_PAL;
        }
    }
    cycleCounter += cycles;
    while (cycleCounter >= cyclesPerSample) {
        cycleCounter -= cyclesPerSample;
        pushSample();
    }
}

void Audio::pushSample() {
    double sample = 0.0;
    sample = emuConsole->getAudioOutput();
    
    std::lock_guard<std::mutex> lock(mutex);
    buffer.push(sample);
    
    if (buffer.size() > 4096) {
        buffer.pop();
    }
}
