#pragma once

#include "literals.hpp"

class Kalman_Filter {
    private:
    float m_angle;
    float m_P;
    float m_Q;
    float m_R;

    public:
    Kalman_Filter()
    : m_angle(0.0f), m_P(1.0f),
      m_Q(0.01f),
      m_R(0.1f) {}

    float update(float gyro_rate, float accel_angle, literals::dura_t dt) {
        m_angle += gyro_rate * dt.v;
        m_P += m_Q;

        float K = m_P / (m_P + m_R);
        m_angle += K * (accel_angle - m_angle);
        m_P = (1 - K) * m_P;

        return m_angle;
    }

    float get_angle() { return m_angle; }
};
