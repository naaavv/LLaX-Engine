#pragma once

#include "Base.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace LLaX
{
    class Log
    {
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };
}

// Core log macros
#define LLAX_CORE_TRACE(...)    ::LLaX::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LLAX_CORE_INFO(...)     ::LLaX::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LLAX_CORE_WARN(...)     ::LLaX::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LLAX_CORE_ERROR(...)    ::LLaX::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LLAX_CORE_CRITICAL(...) ::LLaX::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define LLAX_TRACE(...)         ::LLaX::Log::GetClientLogger()->trace(__VA_ARGS__)
#define LLAX_INFO(...)          ::LLaX::Log::GetClientLogger()->info(__VA_ARGS__)
#define LLAX_WARN(...)          ::LLaX::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LLAX_ERROR(...)         ::LLaX::Log::GetClientLogger()->error(__VA_ARGS__)
#define LLAX_CRITICAL(...)      ::LLaX::Log::GetClientLogger()->critical(__VA_ARGS__)
