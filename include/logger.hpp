// logger.h
#pragma once

#include <Arduino.h>

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_OFF 6

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef LOG_BAUDRATE
#define LOG_BAUDRATE 9600
#endif

#ifndef LOG_SHOW_LEVEL
#define LOG_SHOW_LEVEL true
#endif

#ifndef LOG_SHOW_LOCATION
#define LOG_SHOW_LOCATION true
#endif

namespace logger_detail {
inline void print_formatted_impl(const char* format) {
    while (*format) {
        Serial.print(*format++);
    }
}

template <typename T, typename... Args>
inline void print_formatted_impl(const char* format, T&& value, Args&&... args) {
    while (*format) {
        if (*format == '{' && *(format + 1) == '}') {
            Serial.print(value);
            print_formatted_impl(format + 2, static_cast<Args&&>(args)...);
            return;
        }
        Serial.print(*format++);
    }
}

template <typename... Args>
inline void print_formatted_flash(const __FlashStringHelper* format, Args&&... args) {
    PGM_P p = reinterpret_cast<PGM_P>(format);
    char buffer[256];
    strncpy_P(buffer, p, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    print_formatted_impl(buffer, static_cast<Args&&>(args)...);
}

template <bool ShowLevel, bool ShowLocation>
struct HeaderPrinter {
    static inline void print(const __FlashStringHelper* level,
    const __FlashStringHelper* file,
    int line) {
        if constexpr (ShowLevel) {
            Serial.print('[');
            Serial.print(level);
            Serial.print(F("] "));
        }
        if constexpr (ShowLocation) {
            Serial.print('[');
            Serial.print(file);
            Serial.print(':');
            Serial.print(line);
            Serial.print(F("] "));
        }
    }
};

inline void logger_begin() {
    Serial.begin(LOG_BAUDRATE);
    delay(100);
    auto t = millis();
    while (!Serial.available() && millis() - t < 1000) delay(10);
}

class PendingLog {
    private:
    bool m_active;
    int m_padding_length;

    public:
    PendingLog() : m_active(false), m_padding_length(0) {}

    inline void start(int padding = 10) {
        m_active         = true;
        m_padding_length = padding;
    }

    inline void done(const __FlashStringHelper* status) {
        if (m_active) {
            for (int i = 0; i < m_padding_length; i++) {
                Serial.print('\t');
            }
            Serial.println(status);
            m_active = false;
        }
    }

    inline bool is_active() const { return m_active; }
};

inline PendingLog& get_pending_log() {
    static PendingLog pending;
    return pending;
}
} // namespace logger_detail

#if LOG_LEVEL != LOG_LEVEL_OFF
#define LOG_BEGIN() logger_detail::logger_begin()
#else
#define LOG_BEGIN()
#endif

// TRACE
#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(format, ...)                                                  \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("TRACE"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        Serial.println();                                                       \
    } while (0)

#define LOG_TRACE_START(format, ...)                                            \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("TRACE"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        logger_detail::get_pending_log().start();                               \
    } while (0)
#else
#define LOG_TRACE(format, ...)
#define LOG_TRACE_START(format, ...)

#endif

// DEBUG
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...)                                                  \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("DEBUG"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        Serial.println();                                                       \
    } while (0)

#define LOG_DEBUG_START(format, ...)                                            \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("DEBUG"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        logger_detail::get_pending_log().start();                               \
    } while (0)
#else
#define LOG_DEBUG(format, ...)
#define LOG_DEBUG_START(format, ...)

#endif

// INFO
#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ...)                                                   \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("INFO "), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        Serial.println();                                                       \
    } while (0)

#define LOG_INFO_START(format, ...)                                             \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("INFO "), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        logger_detail::get_pending_log().start();                               \
    } while (0)
#else
#define LOG_INFO(format, ...)
#define LOG_INFO_START(format, ...)

#endif

// WARN
#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ...)                                                   \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("WARN "), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        Serial.println();                                                       \
    } while (0)

#define LOG_WARN_START(format, ...)                                             \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("WARN "), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        logger_detail::get_pending_log().start();                               \
    } while (0)
#else
#define LOG_WARN(format, ...)
#define LOG_WARN_START(format, ...)

#endif

// ERROR
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...)                                                  \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("ERROR"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        Serial.println();                                                       \
    } while (0)

#define LOG_ERROR_START(format, ...)                                            \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("ERROR"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        logger_detail::get_pending_log().start();                               \
    } while (0)
#else
#define LOG_ERROR(format, ...)
#define LOG_ERROR_START(format, ...)

#endif

// FATAL
#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define LOG_FATAL(format, ...)                                                  \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("FATAL"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        Serial.println();                                                       \
    } while (0)

#define LOG_FATAL_START(format, ...)                                            \
    do {                                                                        \
        logger_detail::HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print( \
        F("FATAL"), F(__FILE__), __LINE__);                                     \
        logger_detail::print_formatted_flash(F(format), ##__VA_ARGS__);         \
        logger_detail::get_pending_log().start();                               \
    } while (0)
#else
#define LOG_FATAL(format, ...)
#define LOG_FATAL_START(format, ...)

#endif

#if LOG_LEVEL != LOG_LEVEL_OFF
#define LOG_DONE() logger_detail::get_pending_log().done(F("[DONE]"))
#define LOG_FAIL() logger_detail::get_pending_log().done(F("[FAIL]"))
#define LOG_SKIP() logger_detail::get_pending_log().done(F("[SKIP]"))
#else
#define LOG_DONE()
#define LOG_FAIL()
#define LOG_SKIP()
#endif
