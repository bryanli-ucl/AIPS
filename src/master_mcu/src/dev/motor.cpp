#include "dev/motor.hpp"

#include <Arduino.h>

MotorEncoder::MotorEncoder(uint8_t enc_a, uint8_t enc_b)
: m_pin_enc_a(enc_a), m_pin_enc_b(enc_b),
  m_count(0), m_prev_count(0),
  m_ang_vel(0rad_s),
  m_delta_time(0s), m_prev_time(0) {}

bool MotorEncoder::begin() {
    pinMode(m_pin_enc_a, INPUT_PULLDOWN);
    pinMode(m_pin_enc_b, INPUT_PULLDOWN);

    pinMode(m_pin_en, OUTPUT);
    pinMode(m_pin_dir, OUTPUT);

    attachInterruptParam(digitalPinToInterrupt(m_pin_enc_a), MotorEncoder::isr, CHANGE, this);

    return true;
}

avel_t MotorEncoder::calc_velocity(time_t current_time) {


    noInterrupts();
    int32_t count = m_count;
    interrupts();

    int32_t delta = count - m_prev_count;
    m_prev_count  = count;

    constexpr float GEAR_RATIO    = (22.f / 12.f) * (22.f / 10.f) * (24.f / 10.f);
    constexpr float RAW_CPR       = 48.f;
    constexpr float CPR_REV       = 1 / (GEAR_RATIO * RAW_CPR);
    constexpr float CONSTANT_PART = CPR_REV * TWO_PI * 1000'000.f;

    if (m_prev_time == current_time) {
        m_ang_vel.v = 0;
        return m_ang_vel;
    }

    m_ang_vel.v = delta * CONSTANT_PART / (float)((uint32_t)(current_time - m_prev_time));
    m_prev_time = current_time;
}

void MotorEncoder::isr(void* raw_ins) {

    MotorEncoder* ins = reinterpret_cast<MotorEncoder*>(raw_ins);
    if (digitalRead(ins->m_pin_enc_a) == digitalRead(ins->m_pin_enc_b)) {
        ins->m_count++;
    } else {
        ins->m_count--;
    }
}
