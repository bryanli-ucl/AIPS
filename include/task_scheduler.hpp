#pragma once

#include <Arduino.h>
#include <functional>
#include <stdint.h>


// ============================================================
//  TaskScheduler
//
//  Replaces the raw millis()-polling pattern in loop().
//  Register tasks with an interval; call tick() every loop.
//
//  Usage:
//    TaskScheduler scheduler;
//    scheduler.add(5,    task_5ms);
//    scheduler.add(100,  task_100ms);
//    scheduler.add(500,  task_500ms);
//    scheduler.add(1000, task_1s);
//    scheduler.add(5000, task_5s);
//
//    void loop() { scheduler.tick(); }
//
//  Notes:
//  - Tasks are called synchronously (no preemption).
//  - If a task takes longer than its interval the next call
//    is simply delayed; there is no catch-up accumulation.
//  - MAX_TASKS can be increased if needed.
// ============================================================
class TaskScheduler {
    public:
    static constexpr uint8_t MAX_TASKS = 16;

    using TaskFn = std::function<void()>;

    // --------------------------------------------------------
    //  add()
    //  interval_ms : repeat interval in milliseconds
    //  fn          : callable (free function, lambda, etc.)
    //  Returns false if the task table is full.
    // --------------------------------------------------------
    bool add(uint32_t interval_ms, TaskFn fn) {
        if (m_count >= MAX_TASKS) return false;

        m_tasks[m_count] = {
            .fn          = fn,
            .interval_ms = interval_ms,
            .last_ms     = static_cast<uint32_t>(-1), // fire immediately on first tick
        };
        ++m_count;
        return true;
    }

    // --------------------------------------------------------
    //  tick(ms)  – call from loop(), no delay inside loop needed
    // --------------------------------------------------------
    void tick(uint32_t now_ms) {
        for (uint8_t i = 0; i < m_count; ++i) {
            Task& t = m_tasks[i];
            if (now_ms - t.last_ms >= t.interval_ms) {
                t.last_ms = now_ms;
                t.fn();
            }
        }
    }

    // --------------------------------------------------------
    //  reset()  – restart all task timers
    // --------------------------------------------------------
    void reset() {
        for (uint8_t i = 0; i < m_count; ++i)
            m_tasks[i].last_ms = static_cast<uint32_t>(-1);
    }

    private:
    struct Task {
        TaskFn fn;
        uint32_t interval_ms;
        uint32_t last_ms;
    };

    Task m_tasks[MAX_TASKS]{};
    uint8_t m_count = 0;
};
