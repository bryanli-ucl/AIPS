// motor.hpp
#pragma once

#include <stdint.h>

#include <literals.hpp>
using namespace ::literals;

class Motor {

    public:
    Motor(uint8_t en_a, uint8_t en_b, uint8_t dir, uint8_t en);

    bool begin();

    avel_t calc_velocity();

    avel_t get_avel() { return m_ang_vel; }

    static void update_time();

    private:
    static void isr(void*);

    private:
    uint8_t m_pin_enc_a;
    uint8_t m_pin_enc_b;
    uint8_t m_pin_en;
    uint8_t m_pin_dir;

    int32_t m_count;      // -2^31 -- 2^31-1
    int32_t m_prev_count; // -2^31 -- 2^31-1

    avel_t m_ang_vel;
    static dura_t m_delta_time;
    static time_t m_prev_time;
};
