#include "dev/motor.hpp"

#include <Arduino.h>

dura_t Motor::m_delta_time = 0us;
time_t Motor::m_prev_time  = 0;

Motor::Motor(uint8_t enc_a, uint8_t enc_b, uint8_t dir, uint8_t en)
: m_pin_enc_a(enc_a), m_pin_enc_b(enc_b),
  m_pin_en(en), m_pin_dir(dir),
  m_count(0), m_prev_count(0),
  m_ang_vel(0rad_s) {}

bool Motor::begin() {
    pinMode(m_pin_enc_a, INPUT_PULLDOWN);
    pinMode(m_pin_enc_b, INPUT_PULLDOWN);

    pinMode(m_pin_en, OUTPUT);
    pinMode(m_pin_dir, OUTPUT);

    attachInterruptParam(digitalPinToInterrupt(m_pin_enc_a), Motor::isr, CHANGE, this);

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
    if (digitalRead(ins->m_pin_enc_a) == digitalRead(ins->m_pin_enc_b)) {
        ins->m_count++;
    } else {
        ins->m_count--;
    }
}
