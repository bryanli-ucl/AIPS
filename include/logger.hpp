// logger_optimized.h
// 零成本抽象的 Arduino 日志系统
// 特性：
// 1. 禁用的日志级别完全零开销（编译期消除）
// 2. 无栈缓冲区（逐字符从 Flash 读取）
// 3. 参数惰性求值（禁用时不执行）
// 4. 最小化模板实例化

#pragma once

#include <Arduino.h>

// ============================================================================
// 日志级别定义
// ============================================================================

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_FATAL 5
#define LOG_LEVEL_OFF 6

// 配置选项
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef LOG_BAUDRATE
#define LOG_BAUDRATE 115200
#endif

#ifndef LOG_SHOW_LEVEL
#define LOG_SHOW_LEVEL true
#endif

#ifndef LOG_SHOW_LOCATION
#define LOG_SHOW_LOCATION true
#endif

namespace logger_detail {


constexpr size_t strlen_ce(const char* str) {
    return *str ? 1 + strlen_ce(str + 1) : 0;
}

constexpr void strcpy_ce(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// 从 Flash 逐字符读取并打印，直到遇到 {} 占位符或字符串结束
inline bool print_until_placeholder(PGM_P& format) {
    char c;
    while ((c = pgm_read_byte(format))) {
        if (c == '{' && pgm_read_byte(format + 1) == '}') {
            format += 2; // 跳过 {}
            return true; // 找到占位符
        }
        Serial.write(c);
        format++;
    }
    return false; // 字符串结束
}

// 打印剩余的格式字符串（无占位符）
inline void print_remaining(PGM_P format) {
    char c;
    while ((c = pgm_read_byte(format++))) {
        Serial.write(c);
    }
}

// 递归终止：没有更多参数
inline void format_print_impl(PGM_P format) {
    print_remaining(format);
}

template <typename T, typename... Args>
inline void format_print_impl(PGM_P format, T&& value, Args&&... args) {
    if (print_until_placeholder(format)) {
        Serial.print(static_cast<T&&>(value));
        format_print_impl(format, static_cast<Args&&>(args)...);
    }
}

template <typename... Args>
inline void format_print(const __FlashStringHelper* flash_format, Args&&... args) {
    PGM_P format = reinterpret_cast<PGM_P>(flash_format);
    format_print_impl(format, static_cast<Args&&>(args)...);
}

template <bool ShowLevel, bool ShowLocation>
struct HeaderPrinter {
    template <typename... Args>
    static inline void print_all(const __FlashStringHelper* level, const __FlashStringHelper* file, int line, const __FlashStringHelper* user_format, Args&&... args) {

        if constexpr (ShowLevel && ShowLocation) {
            PGM_P combined_fmt = PSTR("[{}] [{}:{}] ");
            format_print(reinterpret_cast<const __FlashStringHelper*>(combined_fmt), level, file, line);
            format_print(user_format, static_cast<Args&&>(args)...);

        } else if constexpr (ShowLevel && !ShowLocation) {
            PGM_P combined_fmt = PSTR("[{}] ");
            format_print(reinterpret_cast<const __FlashStringHelper*>(combined_fmt), level);
            format_print(user_format, static_cast<Args&&>(args)...);

        } else if constexpr (ShowLocation && !ShowLevel) {
            PGM_P combined_fmt = PSTR("[{}:{}] ");
            format_print(reinterpret_cast<const __FlashStringHelper*>(combined_fmt), file, line);
            format_print(user_format, static_cast<Args&&>(args)...);
        } else {
            format_print(user_format, static_cast<Args&&>(args)...);
        }
    }
};

template <int Level, bool Enabled = (Level >= LOG_LEVEL)>
struct LoggerImpl {
    template <typename... Args>
    static inline void log(const __FlashStringHelper* level, const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args&&... args) {
        HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print_all(level, file, line, format, static_cast<Args&&>(args)...);
        Serial.println();
    }

    template <typename... Args>
    static inline void log_start(const __FlashStringHelper* level, const __FlashStringHelper* file, int line, const __FlashStringHelper* format, Args&&... args) {
        HeaderPrinter<LOG_SHOW_LEVEL, LOG_SHOW_LOCATION>::print_all(level, file, line, format, static_cast<Args&&>(args)...);
    }
};

template <int Level>
struct LoggerImpl<Level, false> {
    template <typename... Args>
    static constexpr inline void log(const __FlashStringHelper*, const __FlashStringHelper*, int, const __FlashStringHelper*, Args&&...) noexcept {}

    template <typename... Args>
    static constexpr inline void log_start(const __FlashStringHelper*, const __FlashStringHelper*, int, const __FlashStringHelper*, Args&&...) noexcept {}
};

#if LOG_LEVEL != LOG_LEVEL_OFF

class PendingLog {
    private:
    bool m_active;
    uint8_t m_padding_count;

    public:
    PendingLog() : m_active(false), m_padding_count(0) {}

    inline void start(uint8_t padding = 10) {
        m_active        = true;
        m_padding_count = padding;
    }

    inline void finish(const __FlashStringHelper* status) {
        if (m_active) {
            for (uint8_t i = 0; i < m_padding_count; i++) {
                Serial.write('\t');
            }
            Serial.println(status);
            m_active = false;
        }
    }

    inline bool is_active() const { return m_active; }
};

inline PendingLog& get_pending_log() {
    static PendingLog instance;
    return instance;
}

#endif

// ----------------------------------------------------------------------------
// Logger 初始化
// ----------------------------------------------------------------------------

inline void logger_init() {
#if LOG_LEVEL != LOG_LEVEL_OFF
    Serial.begin(LOG_BAUDRATE);

    // 等待串口就绪（最多 1 秒）
    unsigned long start = millis();
    while (!Serial && (millis() - start < 1000)) {
        delay(10);
    }
#endif
}

} // namespace logger_detail

// ============================================================================
// 公共 API - 宏定义
// ============================================================================

// 初始化日志系统
#if LOG_LEVEL != LOG_LEVEL_OFF
#define LOG_INIT() logger_detail::logger_init()
#else
#define LOG_INIT() ((void)0)
#endif

// ----------------------------------------------------------------------------
// TRACE 级别
// ----------------------------------------------------------------------------

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(format, ...)                       \
    logger_detail::LoggerImpl<LOG_LEVEL_TRACE>::log( \
    F("TRACE"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)

#define LOG_TRACE_START(format, ...)                                  \
    do {                                                              \
        logger_detail::LoggerImpl<LOG_LEVEL_TRACE>::log_start(        \
        F("TRACE"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__); \
        logger_detail::get_pending_log().start();                     \
    } while (0)
#else
#define LOG_TRACE(format, ...) ((void)0)
#define LOG_TRACE_START(format, ...) ((void)0)
#endif

// ----------------------------------------------------------------------------
// DEBUG 级别
// ----------------------------------------------------------------------------

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...)                       \
    logger_detail::LoggerImpl<LOG_LEVEL_DEBUG>::log( \
    F("DEBUG"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)

#define LOG_DEBUG_START(format, ...)                                  \
    do {                                                              \
        logger_detail::LoggerImpl<LOG_LEVEL_DEBUG>::log_start(        \
        F("DEBUG"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__); \
        logger_detail::get_pending_log().start();                     \
    } while (0)
#else
#define LOG_DEBUG(format, ...) ((void)0)
#define LOG_DEBUG_START(format, ...) ((void)0)
#endif

// ----------------------------------------------------------------------------
// INFO 级别
// ----------------------------------------------------------------------------

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ...)                       \
    logger_detail::LoggerImpl<LOG_LEVEL_INFO>::log( \
    F("INFO "), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)

#define LOG_INFO_START(format, ...)                                   \
    do {                                                              \
        logger_detail::LoggerImpl<LOG_LEVEL_INFO>::log_start(         \
        F("INFO "), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__); \
        logger_detail::get_pending_log().start();                     \
    } while (0)
#else
#define LOG_INFO(format, ...) ((void)0)
#define LOG_INFO_START(format, ...) ((void)0)
#endif

// ----------------------------------------------------------------------------
// WARN 级别
// ----------------------------------------------------------------------------

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ...)                       \
    logger_detail::LoggerImpl<LOG_LEVEL_WARN>::log( \
    F("WARN "), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)

#define LOG_WARN_START(format, ...)                                   \
    do {                                                              \
        logger_detail::LoggerImpl<LOG_LEVEL_WARN>::log_start(         \
        F("WARN "), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__); \
        logger_detail::get_pending_log().start();                     \
    } while (0)
#else
#define LOG_WARN(format, ...) ((void)0)
#define LOG_WARN_START(format, ...) ((void)0)
#endif

// ----------------------------------------------------------------------------
// ERROR 级别
// ----------------------------------------------------------------------------

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...)                       \
    logger_detail::LoggerImpl<LOG_LEVEL_ERROR>::log( \
    F("ERROR"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)

#define LOG_ERROR_START(format, ...)                                  \
    do {                                                              \
        logger_detail::LoggerImpl<LOG_LEVEL_ERROR>::log_start(        \
        F("ERROR"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__); \
        logger_detail::get_pending_log().start();                     \
    } while (0)
#else
#define LOG_ERROR(format, ...) ((void)0)
#define LOG_ERROR_START(format, ...) ((void)0)
#endif

// ----------------------------------------------------------------------------
// FATAL 级别
// ----------------------------------------------------------------------------

#if LOG_LEVEL <= LOG_LEVEL_FATAL
#define LOG_FATAL(format, ...)                       \
    logger_detail::LoggerImpl<LOG_LEVEL_FATAL>::log( \
    F("FATAL"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__)

#define LOG_FATAL_START(format, ...)                                  \
    do {                                                              \
        logger_detail::LoggerImpl<LOG_LEVEL_FATAL>::log_start(        \
        F("FATAL"), F(__FILE__), __LINE__, F(format), ##__VA_ARGS__); \
        logger_detail::get_pending_log().start();                     \
    } while (0)
#else
#define LOG_FATAL(format, ...) ((void)0)
#define LOG_FATAL_START(format, ...) ((void)0)
#endif

// ----------------------------------------------------------------------------
// Pending Log 完成宏
// ----------------------------------------------------------------------------

#if LOG_LEVEL != LOG_LEVEL_OFF
#define LOG_DONE() logger_detail::get_pending_log().finish(F("[DONE]"))
#define LOG_FAIL() logger_detail::get_pending_log().finish(F("[FAIL]"))
#define LOG_SKIP() logger_detail::get_pending_log().finish(F("[SKIP]"))
#define LOG_OK() logger_detail::get_pending_log().finish(F("[ OK ]"))
#else
#define LOG_DONE() ((void)0)
#define LOG_FAIL() ((void)0)
#define LOG_SKIP() ((void)0)
#define LOG_OK() ((void)0)
#endif

// ============================================================================
// 高级功能宏
// ============================================================================

// 条件日志
#define LOG_DEBUG_IF(condition, format, ...)  \
    do {                                      \
        if (condition) {                      \
            LOG_DEBUG(format, ##__VA_ARGS__); \
        }                                     \
    } while (0)

#define LOG_INFO_IF(condition, format, ...)  \
    do {                                     \
        if (condition) {                     \
            LOG_INFO(format, ##__VA_ARGS__); \
        }                                    \
    } while (0)

#define LOG_WARN_IF(condition, format, ...)  \
    do {                                     \
        if (condition) {                     \
            LOG_WARN(format, ##__VA_ARGS__); \
        }                                    \
    } while (0)

#define LOG_ERROR_IF(condition, format, ...)  \
    do {                                      \
        if (condition) {                      \
            LOG_ERROR(format, ##__VA_ARGS__); \
        }                                     \
    } while (0)

// 断言宏
#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ASSERT(condition, format, ...)                         \
    do {                                                           \
        if (!(condition)) {                                        \
            LOG_ERROR("Assertion failed: " format, ##__VA_ARGS__); \
            while (1) { delay(100); } /* 死循环 */              \
        }                                                          \
    } while (0)
#else
#define LOG_ASSERT(condition, format, ...) ((void)0)
#endif

// 十六进制转储（用于调试二进制数据）
#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_HEX_DUMP(data, length)                               \
    do {                                                         \
        LOG_DEBUG("Hex dump ({} bytes):", length);               \
        for (size_t i = 0; i < (length); i++) {                  \
            if (i % 16 == 0) Serial.print(F("\n  "));            \
            if (((uint8_t*)(data))[i] < 0x10) Serial.print('0'); \
            Serial.print(((uint8_t*)(data))[i], HEX);            \
            Serial.print(' ');                                   \
        }                                                        \
        Serial.println();                                        \
    } while (0)
#else
#define LOG_HEX_DUMP(data, length) ((void)0)
#endif