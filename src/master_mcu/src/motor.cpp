#include "motor.hpp"

#include <Arduino.h>

Motor::Motor(uint8_t enc_a, uint8_t enc_b, uint8_t motor_num, MotoronI2C& motoron)
: m_pin_enc_a(enc_a), m_pin_enc_b(enc_b),
  m_motor_num(motor_num),
  m_count(0), m_prev_count(0),
  m_target_avel(0rad_s),
  m_vel_pid(),
  m_motoron(motoron),
  m_ang_vel(0rad_s),
  m_power(0),
  m_delta_time(0s), m_prev_time(0) {}

bool Motor::begin() {
    pinMode(m_pin_enc_a, INPUT_PULLDOWN);
    pinMode(m_pin_enc_b, INPUT_PULLDOWN);


    m_vel_pid.set_paras({ 10.f, .4f, 40.f });

    attachInterruptParam(digitalPinToInterrupt(m_pin_enc_a), Motor::isr, CHANGE, this);

    return true;
}

avel_t Motor::calc_velocity(dura_t current_time) {


    noInterrupts();
    int32_t count = m_count;
    interrupts();

    int32_t delta = count - m_prev_count;
    m_prev_count  = count;

    constexpr float GEAR_RATIO    = (22.f / 12.f) * (22.f / 10.f) * (24.f / 10.f);
    constexpr float RAW_CPR       = 48.f;
    constexpr float CPR_REV       = 1 / (GEAR_RATIO * RAW_CPR);
    constexpr float CONSTANT_PART = CPR_REV * TWO_PI;

    if (m_prev_time == current_time) {
        m_ang_vel.v = 0;
        return m_ang_vel;
    }

    m_ang_vel   = delta * CONSTANT_PART / (current_time - m_prev_time);
    m_prev_time = current_time;
}

void Motor::update_power(dura_t current_time) {

    m_power += m_vel_pid.update(m_ang_vel.v, current_time);

    m_motoron.setSpeed(m_motor_num, m_power);
}

void Motor::isr(void* raw_ins) {

    Motor* ins = reinterpret_cast<Motor*>(raw_ins);
    if (digitalRead(ins->m_pin_enc_a) == digitalRead(ins->m_pin_enc_b)) {
        ins->m_count++;
    } else {
        ins->m_count--;
    }
}
