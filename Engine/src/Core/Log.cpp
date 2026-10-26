#include "Log.h"
#include "LogSink.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>

namespace LLaX
{
    Ref<spdlog::logger> Log::s_CoreLogger;
    Ref<spdlog::logger> Log::s_ClientLogger;

    void Log::Init()
    {
        // ── Ensure the logs directory exists ────────────────────────────────
        std::filesystem::create_directories("logs");

        // ── Sinks ────────────────────────────────────────────────────────────
        // 1. Colorized console sink  (Debug builds only — removed in Release)
        // 2. Rotating file sink: keeps logs/LLaX.log, max 5 MB, 3 rotated files
        // 3. In-memory editor console sink for ImGui Console Panel
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_pattern("%^[%H:%M:%S.%e] [%l] %n: %v%$");

        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            "logs/LLaX.log",
            5 * 1024 * 1024,  // 5 MB per file
            3                  // keep 3 rotated files
        );
        fileSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%n] %v");

        auto editorSink = std::make_shared<EditorLogSink>(1000);
        editorSink->set_pattern("%v");

        // ── Core logger (Engine internals) ───────────────────────────────────
        s_CoreLogger = std::make_shared<spdlog::logger>(
            "LLaX",
            spdlog::sinks_init_list{ consoleSink, fileSink, editorSink }
        );
        s_CoreLogger->set_level(spdlog::level::trace);
        s_CoreLogger->flush_on(spdlog::level::trace);

        // ── Client logger (Sandbox / game code) ─────────────────────────────
        s_ClientLogger = std::make_shared<spdlog::logger>(
            "APP",
            spdlog::sinks_init_list{ consoleSink, fileSink, editorSink }
        );
        s_ClientLogger->set_level(spdlog::level::trace);
        s_ClientLogger->flush_on(spdlog::level::trace);

        // Register with spdlog's global registry so they survive scope changes
        spdlog::register_logger(s_CoreLogger);
        spdlog::register_logger(s_ClientLogger);

        LLAX_CORE_INFO("─────────────────────────────────────────");
        LLAX_CORE_INFO("  LLaX Engine v0.1.0  |  Log initialized");
        LLAX_CORE_INFO("  Console + File (logs/LLaX.log)");
        LLAX_CORE_INFO("─────────────────────────────────────────");
    }

    void Log::Shutdown()
    {
        LLAX_CORE_INFO("Log system shutting down.");
        spdlog::shutdown(); // flushes all sinks and destroys registered loggers
    }
}
