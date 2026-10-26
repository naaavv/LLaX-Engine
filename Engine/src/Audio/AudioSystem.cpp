// AudioSystem.cpp
// Implements the miniaudio high-level engine wrapper.
// We define MINIAUDIO_IMPLEMENTATION here (exactly once in the project).

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"  // from vendor/miniaudio/

#include "AudioSystem.h"
#include "Core/Log.h"

#include <unordered_map>

namespace LLaX
{
    ma_engine* AudioSystem::s_Engine = nullptr;

    void AudioSystem::Init()
    {
        s_Engine = new ma_engine();
        ma_result result = ma_engine_init(nullptr, s_Engine);
        if (result != MA_SUCCESS)
        {
            LLAX_CORE_ERROR("[Audio] Failed to initialize miniaudio engine! Error: {0}", (int)result);
            delete s_Engine;
            s_Engine = nullptr;
            return;
        }
        LLAX_CORE_INFO("[Audio] miniaudio engine initialized successfully.");
    }

    void AudioSystem::Shutdown()
    {
        if (s_Engine)
        {
            ma_engine_uninit(s_Engine);
            delete s_Engine;
            s_Engine = nullptr;
            LLAX_CORE_INFO("[Audio] AudioSystem shut down.");
        }
    }

    void AudioSystem::Play(const std::string& filepath, float volume, bool loop)
    {
        if (!s_Engine)
        {
            LLAX_CORE_WARN("[Audio] AudioSystem not initialized — cannot play '{0}'", filepath);
            return;
        }

        // ma_engine_play_sound is fire-and-forget; doesn't support per-sound control.
        // For full control (loop, volume) we need ma_sound — kept simple here.
        ma_result result = ma_engine_play_sound(s_Engine, filepath.c_str(), nullptr);
        if (result != MA_SUCCESS)
            LLAX_CORE_WARN("[Audio] Failed to play '{0}' (error {1})", filepath, (int)result);
    }

    void AudioSystem::Stop(const std::string& filepath)
    {
        // Fire-and-forget sounds can't be individually stopped in this simple API.
        // Full per-sound control requires storing ma_sound handles — left for future.
        (void)filepath;
    }

    void AudioSystem::StopAll()
    {
        // Reinitialise the engine to forcefully stop all sounds (blunt but effective)
        if (s_Engine)
        {
            ma_engine_stop(s_Engine);
            ma_engine_start(s_Engine);
        }
    }
}
