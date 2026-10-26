#pragma once

#include "Core/Base.h"
#include "Core/Log.h"
#include <string>
#include <unordered_map>

// Forward declare to avoid pulling miniaudio into every header
struct ma_engine;

namespace LLaX
{
    // ─────────────────────────────────────────────────────────────────────────
    //  AudioSystem  — thin singleton wrapper around miniaudio's high-level API
    // ─────────────────────────────────────────────────────────────────────────
    class AudioSystem
    {
    public:
        static void Init();
        static void Shutdown();

        // Play a sound file once (fire-and-forget)
        static void Play(const std::string& filepath, float volume = 1.0f, bool loop = false);
        static void Stop(const std::string& filepath);
        static void StopAll();

        static bool IsInitialized() { return s_Engine != nullptr; }
        static ma_engine* GetEngine() { return s_Engine; }

    private:
        static ma_engine* s_Engine;
    };
}
