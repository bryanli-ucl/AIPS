#pragma once

#include <Arduino.h>
#include <functional>
#include <stdint.h>

class TaskScheduler {
    public:
    static constexpr uint8_t MAX_TASKS = 8;

    using TaskFn = std::function<void()>;

    bool add(uint32_t interval_ms, TaskFn fn, const char* name = "Untitled") {

        if (interval_ms == static_cast<uint32_t>(-1)) return false;
        if (m_count >= MAX_TASKS) return false;

        m_tasks[m_count] = {
            .fn          = fn,
            .name        = name,
            .interval_ms = interval_ms,
            // fire immediately on first tick
            .last_ms = static_cast<uint32_t>(-1),
            .busy_us = 0,
        };
        m_count++;
        return true;
    }

    void tick(uint32_t now_ms) {
        for (uint8_t i = 0; i < m_count; ++i) {
            Task& t = m_tasks[i];
            if (now_ms - t.last_ms >= t.interval_ms) {
                t.last_ms   = now_ms;
                uint32_t t0 = micros();
                t.fn();
                t.busy_us += micros() - t0;
            }
        }
        m_total_us += micros() - m_last_tick_us;
        m_last_tick_us = micros();
    }

    void print_cpu_usage() {
        if (m_total_us == 0) return;

        LOG_DEBUG("CPU USAGE");

        uint32_t sum_busy = 0;
        for (uint8_t i = 0; i < m_count; ++i) {
            uint32_t pct_x100 = (uint32_t)((uint64_t)m_tasks[i].busy_us * 10000 / m_total_us);
            sum_busy += m_tasks[i].busy_us;
            LOG_DEBUG("     [{}]     {}.{}%", m_tasks[i].name, pct_x100 / 100, pct_x100 % 100);
        }
        uint32_t total_x100 = (uint32_t)((uint64_t)sum_busy * 10000 / m_total_us);
        LOG_DEBUG("     [Total]      {}.{}%", total_x100 / 100, total_x100 % 100);
        reset_stats();
    }

    void reset() {
        for (uint8_t i = 0; i < m_count; ++i)
            m_tasks[i].last_ms = static_cast<uint32_t>(-1);
        reset_stats();
    }

    private:
    struct Task {
        TaskFn fn;
        const char* name;
        uint32_t interval_ms;
        uint32_t last_ms;
        uint32_t busy_us;
    };

    void reset_stats() {
        m_total_us     = 0;
        m_last_tick_us = micros();
        for (uint8_t i = 0; i < m_count; ++i)
            m_tasks[i].busy_us = 0;
    }

    Task m_tasks[MAX_TASKS]{};
    uint8_t m_count         = 0;
    uint32_t m_total_us     = 0;
    uint32_t m_last_tick_us = 0;
};
