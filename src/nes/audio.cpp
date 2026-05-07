#include "audio.hpp"
#include "nes.hpp"
#include "nes_apu.hpp"

Audio audioSystem;

void SDLCALL audioCallback(void* userdata, uint8_t* stream, int len) {
    Audio* audio = (Audio*)(userdata);
    int16_t* out = (int16_t*)(stream);
    int samplesRequested = len / sizeof(int16_t);
    
    std::lock_guard<std::mutex> lock(audio->mutex);
    
    for (int i = 0; i < samplesRequested; ++i) {
        double s = 0.0;
        if (!audio->buffer.empty()) {
            s = audio->buffer.front();
            audio->buffer.pop();
        }
        
        out[i] = (int16_t)(s * 30000.0);
    }
}

void Audio::init() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        SDL_Log("Failed to initialize SDL audio: %s", SDL_GetError());
        return;
    }
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = sampleRate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 512;
    want.callback = audioCallback;
    want.userdata = this;
    
    device = SDL_OpenAudioDevice(nullptr, 0, &want, &have, 0);
    if (device == 0) {
        SDL_Log("Failed to open audio device: %s", SDL_GetError());
        return;
    }
    
    SDL_PauseAudioDevice(device, 0);
    DebugPrintLog("APU", "Initialized SDL2 Audio System");
}


void Audio::close() {
    if (device) {
        SDL_CloseAudioDevice(device);
        device = 0;
    }
    DebugPrintLog("APU", "Closed SDL2 Audio System");
}

void Audio::advance() {
    cycleCounter += 1;
    while (cycleCounter >= cyclesPerSample) {
        cycleCounter -= cyclesPerSample;
        pushSample();
    }
}

void Audio::pushSample() {
    double sample = 0.0;
    sample = apu.getOutputSample();
    
    std::lock_guard<std::mutex> lock(mutex);
    buffer.push(sample);
    
    if (buffer.size() > 4096) {
        buffer.pop();
    }
}
