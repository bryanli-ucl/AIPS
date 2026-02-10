// motor.hpp
#pragma once

#include <stdint.h>

#include <Motoron.h>

#include "pid_controller.hpp"

#include "literals.hpp"
#include "serial_logger.hpp"

using namespace ::literals;


class Motor {

    public:
    Motor(uint8_t en_a, uint8_t en_b, uint8_t motor_num, MotoronI2C& motoron);

    bool begin();

    avel_t calc_velocity(dura_t current_time);
    void update_power(dura_t current_time);

    void set_target_avel(avel_t v) { m_target_avel = v; }
    avel_t get_target_avel() { return m_target_avel; }
    avel_t get_avel() { return m_ang_vel; }
    int32_t get_count() { return m_count; }

    private:
    static void isr(void*);

    private:
    ctrl::pid_controller m_vel_pid;
    MotoronI2C& m_motoron;

    uint8_t m_pin_enc_a;
    uint8_t m_pin_enc_b;
    uint8_t m_motor_num;

    volatile int32_t m_count;
    volatile int32_t m_prev_count;

    avel_t m_target_avel;

    int16_t m_power;
    avel_t m_ang_vel;
    dura_t m_delta_time;
    dura_t m_prev_time;
};
