//MIT License

//Copyright (c) 2026 Z-Multiplier
#ifndef AUDIO_HPP
#define AUDIO_HPP
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <stdexcept>
#include <mutex>
#include <windows.h>
#include <atomic>
#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <avrt.h>
#include <algorithm>
#include <cstring>
namespace Audio{
    struct Noise{
        std::wstring path;
        std::vector<float> cache;
        unsigned int sampleRate;
        Noise()=delete;
        Noise(std::wstring path);
    };
    struct PlayRequest{
        std::shared_ptr<Noise> target;
        float volume;
        float pitch;
        bool removeAfterPlay;
        PlayRequest(std::shared_ptr<Noise> tgt,float volume,float pitch,bool removeFlag):target(tgt),volume(volume),pitch(pitch),removeAfterPlay(removeFlag){}
    };
    struct AudioManager{
        private:
            std::atomic<bool> active;
            std::queue<PlayRequest> requestQueue;
            std::thread audioThread;
            std::condition_variable queueCV;
            std::mutex audioMutex;
            AudioManager();
        public:
            static AudioManager& instance();
            ~AudioManager();
            AudioManager(AudioManager&)=delete;
            AudioManager operator=(AudioManager&)=delete;
            AudioManager(AudioManager&&)=delete;
            AudioManager operator=(AudioManager&&)=delete;
            void start();
            void stop();
            void pushRequest(const PlayRequest& request);
            void audioThreadFunc();
    };
}

#endif