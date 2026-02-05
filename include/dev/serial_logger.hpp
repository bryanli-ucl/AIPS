// serial_logger.hpp
#pragma once

#include <Arduino.h>

#include "literals.hpp"
#include "singleton.hpp"

using namespace ::literals;

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_OFF 6

#ifndef LOG_LEVEL
#    define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef LOG_BAUDRATE
#    define LOG_BAUDRATE 115200
#endif

#ifndef LOG_SHOW_LEVEL
#    define LOG_SHOW_LEVEL true
#endif

#ifndef LOG_SHOW_LOCATION
#    define LOG_SHOW_LOCATION true
#endif

namespace __details {

using f_ptr = const __FlashStringHelper*;

template <typename... Args>
constexpr void print(f_ptr fmt, Args... args) {
    print_impl(reinterpret_cast<PGM_P>(fmt), (size_t)0, args...);
}


template <int M, int L, int T>
inline void print_quantity(Quantity<M, L, T> val) {
    Serial.print(val.v);
}

inline void print_done(f_ptr str, size_t gape = 10) {
    while (gape--) {
        Serial.print('\t');
    }
    Serial.println(str);
}

inline void print_impl(PGM_P fmt, size_t idx) {
    size_t len = 0;
    while (fmt[idx + len] != '\0') len++;
    Serial.write(fmt + idx, len);
}

template <typename T, typename... Args>
inline void print_impl(PGM_P fmt, size_t idx, T&& val, Args... args) {
    size_t len = 0;
    while (fmt[idx + len] != '\0' && fmt[idx + len] != '{') len++;
    Serial.write(fmt + idx, len);
    idx += len;

    if (fmt[idx] == '{') {
        if (fmt[idx + 1] == '}') {
            if constexpr (is_quantity_v<T>) {
                Serial.print(val.v);
            } else {
                Serial.print(val);
            }
            print_impl(fmt, idx + 2, args...);
            return;
        }
        if (fmt[idx + 2] == '}') {

            char c = fmt[idx + 1];

            if constexpr (!is_quantity_v<T>) {
                switch (c) {
                case 'h': Serial.print(val, HEX); break;
                case 'd': Serial.print(val, DEC); break;
                case 'o': Serial.print(val, OCT); break;
                case 'b': Serial.print(val, BIN); break;
                }
            }
            print_impl(fmt, idx + 3, args...);
            return;
        }
    }
}

void print_header(f_ptr level, const f_ptr location);
void begin();

} // namespace __details

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define LOCATION_STR \
    "["__FILE__      \
    ":" TOSTRING(__LINE__) "] "

#if LOG_LEVEL != LOG_LEVEL_OFF
#    define LOG_BEGIN() __details::begin();
#else
#    define LOG_BEGIN()
#endif

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#    define LOG_TRACE(fmt, ...)                                      \
        do {                                                         \
            __details::print_header(F("[TRACE] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
            Serial.println();                                        \
        } while (0)
#    define LOG_TRACE_START(fmt, ...)                                \
        do {                                                         \
            __details::print_header(F("[TRACE] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
        } while (0)
#else
#    define LOG_TRACE(fmt, ...)
#    define LOG_TRACE_START(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#    define LOG_DEBUG(fmt, ...)                                      \
        do {                                                         \
            __details::print_header(F("[DEBUG] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
            Serial.println();                                        \
        } while (0)
#    define LOG_DEBUG_START(fmt, ...)                                \
        do {                                                         \
            __details::print_header(F("[DEBUG] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
        } while (0)
#else
#    define LOG_DEBUG(fmt, ...)
#    define LOG_DEBUG_START(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#    define LOG_INFO(fmt, ...)                                      \
        do {                                                        \
            __details::print_header(F("[INFO] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                \
            Serial.println();                                       \
        } while (0)
#    define LOG_INFO_START(fmt, ...)                                \
        do {                                                        \
            __details::print_header(F("[INFO] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                \
        } while (0)
#else
#    define LOG_INFO(fmt, ...)
#    define LOG_INFO_START(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#    define LOG_WARN(fmt, ...)                                      \
        do {                                                        \
            __details::print_header(F("[WARN] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                \
            Serial.println();                                       \
        } while (0)
#    define LOG_WARN_START(fmt, ...)                                \
        do {                                                        \
            __details::print_header(F("[WARN] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                \
        } while (0)
#else
#    define LOG_WARN(fmt, ...)
#    define LOG_WARN_START(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#    define LOG_ERROR(fmt, ...)                                      \
        do {                                                         \
            __details::print_header(F("[ERROR] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
            Serial.println();                                        \
        } while (0)
#    define LOG_ERROR_START(fmt, ...)                                \
        do {                                                         \
            __details::print_header(F("[ERROR] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
        } while (0)
#else
#    define LOG_ERROR(fmt, ...)
#    define LOG_ERROR_START(fmt, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#    define LOG_FATAL(fmt, ...)                                      \
        do {                                                         \
            __details::print_header(F("[FATAL] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
        } while (0)
#    define LOG_FATAL_START(fmt, ...)                                \
        do {                                                         \
            __details::print_header(F("[FATAL] "), F(LOCATION_STR)); \
            __details::print(F(fmt), ##__VA_ARGS__);                 \
        } while (0)
#else
#    define LOG_FATAL(fmt, ...)
#    define LOG_FATAL_START(fmt, ...)
#endif


#if LOG_LEVEL != LOG_LEVEL_OFF
#    define LOG_DONE() __details::print_done(F("[DONE]"))
#    define LOG_FAIL() __details::print_done(F("[FAIL]"))
#    define LOG_SKIP() __details::print_done(F("[SKIP]"))
#else
#    define LOG_DONE()
#    define LOG_FAIL()
#    define LOG_SKIP()
#endif