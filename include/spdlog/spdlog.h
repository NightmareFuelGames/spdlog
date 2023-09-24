// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

// spdlog main header file.
// see example.cpp for usage example

#ifndef SPDLOG_H
#define SPDLOG_H

#pragma once

#include <spdlog/common.h>
#include <spdlog/details/registry.h>
#include <spdlog/logger.h>
#include <spdlog/version.h>
#include <spdlog/details/synchronous_factory.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

namespace spdlog {

using default_factory = synchronous_factory;

// Create and register a logger with a templated sink type
// The logger's level, formatter and flush level will be set according the
// global settings.
//
// Example:
//   spdlog::create<daily_file_sink_st>("logger_name", "dailylog_filename", 11, 59);
template<typename Sink, typename... SinkArgs>
inline std::shared_ptr<spdlog::logger> create(std::string logger_name, SinkArgs &&...sink_args)
{
    return default_factory::create<Sink>(std::move(logger_name), std::forward<SinkArgs>(sink_args)...);
}

// Initialize and register a logger,
// formatter and flush level will be set according the global settings.
//
// Useful for initializing manually created loggers with the global settings.
//
// Example:
//   auto mylogger = std::make_shared<spdlog::logger>("mylogger", ...);
//   spdlog::initialize_logger(mylogger);
SPDLOG_API void initialize_logger(std::shared_ptr<logger> logger);

// Return an existing logger or nullptr if a logger with such name doesn't
// exist.
// example: spdlog::get("my_logger")->info("hello {}", "world");
SPDLOG_API std::shared_ptr<logger> get(const std::string &name);

// Set global formatter. Each sink in each logger will get a clone of this object
SPDLOG_API void set_formatter(std::unique_ptr<spdlog::formatter> formatter);

// Set global format string.
// example: spdlog::set_pattern("%Y-%m-%d %H:%M:%S.%e %l : %v");
SPDLOG_API void set_pattern(std::string pattern, pattern_time_type time_type = pattern_time_type::local);

// Get global logging level
SPDLOG_API level get_level();

// Set global logging level
SPDLOG_API void set_level(level level);

// Determine whether the default logger should log messages with a certain level
SPDLOG_API bool should_log(level lvl);

// Set global flush level
SPDLOG_API void flush_on(level level);

// Start/Restart a periodic flusher thread
// Warning: Use only if all your loggers are thread safe!
template<typename Rep, typename Period>
inline void flush_every(std::chrono::duration<Rep, Period> interval)
{
    details::registry::instance().flush_every(interval);
}

// Set global error handler
SPDLOG_API void set_error_handler(void (*handler)(const std::string &msg));

// Register the given logger with the given name
SPDLOG_API void register_logger(std::shared_ptr<logger> logger);

// Apply a user defined function on all registered loggers
// Example:
// spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) {l->flush();});
SPDLOG_API void apply_all(const std::function<void(std::shared_ptr<logger>)> &fun);

// Drop the reference to the given logger
SPDLOG_API void drop(const std::string &name);

// Drop all references from the registry
SPDLOG_API void drop_all();

// stop any running threads started by spdlog and clean registry loggers
SPDLOG_API void shutdown();

// Automatic registration of loggers when using spdlog::create() or spdlog::create_async
SPDLOG_API void set_automatic_registration(bool automatic_registration);

// API for using default logger (stdout_color_mt),
// e.g: spdlog::info("Message {}", 1);
//
// The default logger object can be accessed using the spdlog::default_logger():
// For example, to add another sink to it:
// spdlog::default_logger()->sinks().push_back(some_sink);
//
// The default logger can replaced using spdlog::set_default_logger(new_logger).
// For example, to replace it with a file logger.
//
// IMPORTANT:
// The default API is thread safe (for _mt loggers), but:
// set_default_logger() *should not* be used concurrently with the default API.
// e.g do not call set_default_logger() from one thread while calling spdlog::info() from another.

SPDLOG_API std::shared_ptr<spdlog::logger> default_logger();

SPDLOG_API spdlog::logger *default_logger_raw();

SPDLOG_API void set_default_logger(std::shared_ptr<spdlog::logger> default_logger);

// Initialize logger level based on environment configs.
//
// Useful for applying SPDLOG_LEVEL to manually created loggers.
//
// Example:
//   auto mylogger = std::make_shared<spdlog::logger>("mylogger", ...);
//   spdlog::apply_logger_env_levels(mylogger);
SPDLOG_API void apply_logger_env_levels(std::shared_ptr<logger> logger);

template<typename... Args>
inline void log(source_loc source, level lvl, format_string_t<Args...> fmt, Args &&...args)
{
    default_logger_raw()->log(source, lvl, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void log(level lvl, format_string_t<Args...> fmt, Args &&...args)
{
    default_logger_raw()->log(lvl, fmt, std::forward<Args>(args)...);
}

template<typename S, typename = is_convertible_to_sv<S>, typename... Args>
inline void log(source_loc loc, level lvl, S fmt, Args &&...args)
{
    default_logger_raw()->log(loc, lvl, fmt, std::forward<Args>(args)...);
}

template<typename S, typename = is_convertible_to_sv<S>, typename... Args>
inline void log(level lvl, S fmt, Args &&...args)
{
    default_logger_raw()->log(lvl, fmt, std::forward<Args>(args)...);
}

#ifdef SPDLOG_SOURCE_LOCATION
template<typename... Args>
inline void trace(loc_with_fmt fmt, Args &&...args)
{
    log(fmt.loc, level::trace, fmt.fmt_string, std::forward<Args>(args)...);
}

template<typename... Args>
inline void debug(loc_with_fmt fmt, Args &&...args)
{
    log(fmt.loc, level::debug, fmt.fmt_string, std::forward<Args>(args)...);
}

template<typename... Args>
inline void info(loc_with_fmt fmt, Args &&...args)
{
    log(fmt.loc, level::info, fmt.fmt_string, std::forward<Args>(args)...);
}

template<typename... Args>
inline void warn(loc_with_fmt fmt, Args &&...args)
{
    log(fmt.loc, spdlog::level::warn, fmt.fmt_string, std::forward<Args>(args)...);
}

template<typename... Args>
inline void error(loc_with_fmt fmt, Args &&...args)
{
    log(fmt.loc, level::err, fmt.fmt_string, std::forward<Args>(args)...);
}

template<typename... Args>
inline void critical(loc_with_fmt fmt, Args &&...args)
{
    log(fmt.loc, level::critical, fmt.fmt_string, std::forward<Args>(args)...);
}

// log functions with no format string, just string
inline void trace(string_view_t msg, source_loc loc = source_loc::current())
{
    log(loc, level::trace, msg);
}

inline void debug(string_view_t msg, source_loc loc = source_loc::current())
{
    log(loc, level::debug, msg);
}

inline void info(string_view_t msg, source_loc loc = source_loc::current())
{
    log(loc, level::info, msg);
}

inline void warn(string_view_t msg, source_loc loc = source_loc::current())
{
    log(loc, spdlog::level::warn, msg);
}

inline void error(string_view_t msg, source_loc loc = source_loc::current())
{
    log(loc, level::err, msg);
}

inline void critical(string_view_t msg, source_loc loc = source_loc::current())
{
    log(loc, level::critical, msg);
}
#else
template<typename... Args>
inline void trace(format_string_t<Args...> fmt, Args &&...args)
{
    log(level::trace, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void debug(format_string_t<Args...> fmt, Args &&...args)
{
    log(level::debug, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void info(format_string_t<Args...> fmt, Args &&...args)
{
    log(level::info, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void warn(format_string_t<Args...> fmt, Args &&...args)
{
    log(level::warn, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void error(format_string_t<Args...> fmt, Args &&...args)
{
    log(level::err, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
inline void critical(format_string_t<Args...> fmt, Args &&...args)
{
    log(level::critical, fmt, std::forward<Args>(args)...);
}

// log functions with no format string, just string
inline void trace(string_view_t msg)
{
    log(level::trace, msg);
}

inline void debug(string_view_t msg)
{
    log(level::debug, msg);
}

inline void info(string_view_t msg)
{
    log(level::info, msg);
}

inline void warn(string_view_t msg)
{
    log(level::warn, msg);
}

inline void error(string_view_t msg)
{
    log(level::err, msg);
}

inline void critical(string_view_t msg)
{
    log(level::critical, msg);
}
#endif

} // namespace spdlog

//
// enable/disable log calls at compile time according to global level.
//
// define SPDLOG_ACTIVE_LEVEL to one of those (before including spdlog.h):
// SPDLOG_LEVEL_TRACE,
// SPDLOG_LEVEL_DEBUG,
// SPDLOG_LEVEL_INFO,
// SPDLOG_LEVEL_WARN,
// SPDLOG_LEVEL_ERROR,
// SPDLOG_LEVEL_CRITICAL,
// SPDLOG_LEVEL_OFF
//

#ifdef SPDLOG_SOURCE_LOCATION
#    define SPDLOG_LOGGER_CALL(logger, level, ...)                                                                                         \
        (logger)->log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_CALL(logger, level, ...) (logger)->log(spdlog::source_loc{}, level, __VA_ARGS__)
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#    define SPDLOG_LOGGER_TRACE(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#    define SPDLOG_TRACE(...) SPDLOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_TRACE(logger, ...) (void)0
#    define SPDLOG_TRACE(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#    define SPDLOG_LOGGER_DEBUG(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::debug, __VA_ARGS__)
#    define SPDLOG_DEBUG(...) SPDLOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_DEBUG(logger, ...) (void)0
#    define SPDLOG_DEBUG(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#    define SPDLOG_LOGGER_INFO(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::info, __VA_ARGS__)
#    define SPDLOG_INFO(...) SPDLOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_INFO(logger, ...) (void)0
#    define SPDLOG_INFO(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#    define SPDLOG_LOGGER_WARN(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::warn, __VA_ARGS__)
#    define SPDLOG_WARN(...) SPDLOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_WARN(logger, ...) (void)0
#    define SPDLOG_WARN(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#    define SPDLOG_LOGGER_ERROR(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#    define SPDLOG_ERROR(...) SPDLOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_ERROR(logger, ...) (void)0
#    define SPDLOG_ERROR(...) (void)0
#endif

#if SPDLOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#    define SPDLOG_LOGGER_CRITICAL(logger, ...) SPDLOG_LOGGER_CALL(logger, spdlog::level::critical, __VA_ARGS__)
#    define SPDLOG_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#    define SPDLOG_LOGGER_CRITICAL(logger, ...) (void)0
#    define SPDLOG_CRITICAL(...) (void)0
#endif

#endif // SPDLOG_H
