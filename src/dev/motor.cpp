#include "dev/motor.hpp"

#include <Arduino.h>

dura_t Motor::m_delta_time = 0us;
time_t Motor::m_prev_time  = 0;

Motor::Motor(uint8_t en_a, uint8_t en_b)
: m_pin_en_a(en_a), m_pin_en_b(en_b),
  m_count(0), m_prev_count(0),
  m_ang_vel(0rad_s) {}

bool Motor::begin() {
    pinMode(m_pin_en_a, INPUT_PULLDOWN);
    pinMode(m_pin_en_b, INPUT_PULLDOWN);

    attachInterruptParam(digitalPinToInterrupt(m_pin_en_a), Motor::isr, CHANGE, this);

    return true;
}

avel_t Motor::calc_velocity() {
    int16_t delta = m_count - m_prev_count;
    m_prev_count  = m_count;

    constexpr float GEAR_RATIO = (22.0 / 12.0) * (22.0 / 10.0) * (24.0 / 10.0);
    constexpr float RAW_CPR    = 48.0;
    constexpr float CPR_REV    = 1 / (GEAR_RATIO * RAW_CPR);

    m_ang_vel = delta * CPR_REV * TWO_PI / m_delta_time;
}

void Motor::update_time() {
    m_delta_time = (micros() - m_prev_time) * 1us;
    m_prev_time  = micros();
}

void Motor::isr(void* raw_ins) {
    Motor* ins = reinterpret_cast<Motor*>(raw_ins);
    if (digitalRead(ins->m_pin_en_a) == digitalRead(ins->m_pin_en_b)) {
        ins->m_count++;
    } else {
        ins->m_count--;
    }
}
