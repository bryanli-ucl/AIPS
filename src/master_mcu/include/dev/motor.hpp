// motor.hpp
#pragma once

#include <stdint.h>

#include "serial_logger.hpp"

#include <literals.hpp>
using namespace ::literals;

class MotorEncoder {

    public:
    MotorEncoder(uint8_t en_a, uint8_t en_b);

    bool begin();

    avel_t calc_velocity(time_t current_time);

    avel_t get_avel() { return m_ang_vel; }
    int32_t get_count() { return m_count; }

    private:
    static void isr(void*);

    private:
    uint8_t m_pin_enc_a;
    uint8_t m_pin_enc_b;
    uint8_t m_pin_en;
    uint8_t m_pin_dir;

    volatile int32_t m_count;
    volatile int32_t m_prev_count;

    avel_t m_ang_vel;
    dura_t m_delta_time;
    time_t m_prev_time;
};
