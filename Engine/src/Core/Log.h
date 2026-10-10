#pragma once

#include "Base.h"

// Pull in spdlog core + fmt formatter support
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace LLaX
{
    class Log
    {
    public:
        // Must be called once at engine startup (Application constructor calls this).
        static void Init();

        // Shut down spdlog — flushes and closes all sinks.
        static void Shutdown();

        // Core logger  — used by engine-internal systems (Window, Renderer, etc.)
        inline static Ref<spdlog::logger>& GetCoreLogger()   { return s_CoreLogger; }

        // Client logger — used by the game / sandbox application code.
        inline static Ref<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

    private:
        static Ref<spdlog::logger> s_CoreLogger;
        static Ref<spdlog::logger> s_ClientLogger;
    };
}

// ─── Core engine log macros ────────────────────────────────────────────────
// Use these inside the Engine library (Window, Renderer, Input, etc.)
#define LLAX_CORE_TRACE(...)    ::LLaX::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LLAX_CORE_INFO(...)     ::LLaX::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LLAX_CORE_WARN(...)     ::LLaX::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LLAX_CORE_ERROR(...)    ::LLaX::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LLAX_CORE_CRITICAL(...) ::LLaX::Log::GetCoreLogger()->critical(__VA_ARGS__)

// ─── Client / application log macros ───────────────────────────────────────
// Use these inside the Sandbox / game application code
#define LLAX_TRACE(...)         ::LLaX::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LLAX_INFO(...)          ::LLaX::Log::GetClientLogger()->info(__VA_ARGS__)
#define LLAX_WARN(...)          ::LLaX::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LLAX_ERROR(...)         ::LLaX::Log::GetClientLogger()->error(__VA_ARGS__)
#define LLAX_CRITICAL(...)      ::LLaX::Log::GetClientLogger()->critical(__VA_ARGS__)
