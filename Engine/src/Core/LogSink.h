#pragma once

#include "Core/Base.h"
#include <spdlog/sinks/base_sink.h>
#include <mutex>
#include <vector>
#include <string>

namespace LLaX
{
    struct LogMessage
    {
        std::string Timestamp;
        std::string Message;
        spdlog::level::level_enum Level;
        std::string LoggerName;
    };

    class EditorLogSink : public spdlog::sinks::base_sink<std::mutex>
    {
    public:
        EditorLogSink(size_t maxMessages = 1000)
            : m_MaxMessages(maxMessages)
        {
        }

        static const std::vector<LogMessage>& GetMessages()
        {
            std::lock_guard<std::mutex> lock(s_BufferMutex);
            return s_Messages;
        }

        static void Clear()
        {
            std::lock_guard<std::mutex> lock(s_BufferMutex);
            s_Messages.clear();
        }

    protected:
        void sink_it_(const spdlog::details::log_msg& msg) override
        {
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<std::mutex>::formatter_->format(msg, formatted);

            std::lock_guard<std::mutex> lock(s_BufferMutex);
            if (s_Messages.size() >= m_MaxMessages)
            {
                s_Messages.erase(s_Messages.begin());
            }

            // Extract time
            auto time = msg.time;
            auto timeT = std::chrono::system_clock::to_time_t(time);
            tm tmBuf;
#if defined(_WIN32)
            localtime_s(&tmBuf, &timeT);
#else
            localtime_r(&timeT, &tmBuf);
#endif
            char timeStr[32];
            std::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &tmBuf);

            LogMessage logMsg;
            logMsg.Timestamp = timeStr;
            logMsg.Message = fmt::to_string(msg.payload);
            logMsg.Level = msg.level;
            logMsg.LoggerName = fmt::to_string(msg.logger_name);

            s_Messages.push_back(std::move(logMsg));
        }

        void flush_() override
        {
        }

    private:
        size_t m_MaxMessages;
        inline static std::vector<LogMessage> s_Messages;
        inline static std::mutex s_BufferMutex;
    };
}
