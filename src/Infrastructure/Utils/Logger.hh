#pragma once

#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Logger {
    using LoggerType = spdlog::async_logger;

public:
    using Level = spdlog::level::level_enum;
    Logger(Level level,
           const std::string& filepath,
           std::size_t q_max_items = 8192,
           std::size_t thread_count = 1) {
        spdlog::file_event_handlers handlers;
        handlers.after_open = [](spdlog::filename_t filename, std::FILE* fstream) {
            fputs("[Start SAST-Evento-Desktop]\n", fstream);
        };
        handlers.before_close = [](spdlog::filename_t filename, std::FILE* fstream) {
            fputs("[End SAST-Evento-Desktop]\n\n\n\n\n", fstream);
        };
        auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filepath,
                                                                            0,
                                                                            0,
                                                                            false,
                                                                            0,
                                                                            handlers);
        auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        if constexpr (std::is_same_v<LoggerType, spdlog::async_logger>) {
            _threadPool = std::make_shared<spdlog::details::thread_pool>(q_max_items, thread_count);
            _logger = std::make_shared<LoggerType>("evento",
                                                   spdlog::sinks_init_list{fileSink, stdoutSink},
                                                   _threadPool);
        } else {
            _logger = std::make_shared<LoggerType>("evento",
                                                   spdlog::sinks_init_list{fileSink, stdoutSink});
        }

#ifdef EVENTO_RELEASE
        stdoutSink->set_level(Level::off);
#endif
        _logger->set_level(level);
        _logger->set_error_handler(
            [&](const std::string& msg) { _logger->error("*** LOGGER ERROR ***: {}", msg); });
        // spdlog::register_logger(_asyncLogger);

        /* more about pattern:
         * https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
         */
        stdoutSink->set_pattern("\033[36m[%Y-%m-%d %H:%M:%S.%e] \033[92m[%n] \033[0m%^[%l]%$ %v");
        spdlog::set_default_logger(_logger);
    }
    auto& logger() { return _logger; }
    [[nodiscard]] Level logLevel() const { return _logger->level(); }
    void setLogLevel(Level level) { _logger->set_level(level); }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger() { spdlog::shutdown(); }

private:
    std::shared_ptr<LoggerType> _logger;
    std::shared_ptr<spdlog::details::thread_pool> _threadPool;
};