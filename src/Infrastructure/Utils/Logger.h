#pragma once

#include <spdlog/async.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

class Logger {
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
        std::shared_ptr<spdlog::sinks::daily_file_sink_mt> fileSink
            = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filepath, 0, 0, false, 0, handlers);
        std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdoutSink
            = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        _threadPool = std::make_shared<spdlog::details::thread_pool>(q_max_items, thread_count);
        _asyncLogger = std::make_shared<spdlog::async_logger>("evento-logger",
                                                              spdlog::sinks_init_list{fileSink,
                                                                                      stdoutSink},
                                                              _threadPool);

#ifdef EVENTO_RELEASE
        stdoutSink->set_level(Level::off);
#endif
        _asyncLogger->set_level(level);
        _asyncLogger->set_error_handler(
            [&](const std::string& msg) { _asyncLogger->error("*** LOGGER ERROR ***: {}", msg); });
        // spdlog::register_logger(_asyncLogger);

        /* more about pattern:
         * https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
         */
        stdoutSink->set_pattern("\033[36m[%Y-%m-%d %H:%M:%S.%e] \033[92m[%n] \033[0m%^[%l]%$ %v");
        spdlog::set_default_logger(_asyncLogger);
    }
    auto& logger() { return _asyncLogger; }
    [[nodiscard]] Level logLevel() const { return _asyncLogger->level(); }
    void setLogLevel(Level level) { _asyncLogger->set_level(level); }
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    ~Logger() { spdlog::shutdown(); }

private:
    std::shared_ptr<spdlog::async_logger> _asyncLogger;
    std::shared_ptr<spdlog::details::thread_pool> _threadPool;
};