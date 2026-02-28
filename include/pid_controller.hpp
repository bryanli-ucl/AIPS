#pragma once

#include <Arduino.h>

#include "serial_logger.hpp"

#include <math.h>
#include <stdint.h>

#include "literals.hpp"

using namespace ::literals;

class PID_Controller {
    private:
    float m_target;

    float m_kp;
    float m_ki;
    float m_kd;

    float m_int;
    float m_prev_err;

    float m_integral_limit;

    bool m_first_sample;

    public:
    PID_Controller() noexcept
    : m_target(.0),
      m_kp(.0), m_ki(.0), m_kd(.0),
      m_int(.0),
      m_prev_err(.0),
      m_integral_limit(INT32_MAX),
      m_first_sample(true) {
    }

    PID_Controller(double kp, double ki, double kd) noexcept
    : PID_Controller() {
        m_kp = kp;
        m_ki = ki;
        m_kd = kd;
    }

    ~PID_Controller() noexcept = default;

    auto update(float val, dura_t rdt) -> float {
        float dt = rdt.v;
        if (m_first_sample) dt = 0;

        float err = m_target - val;

        // P
        float p = err * m_kp;

        // I
        m_int += err * dt;
        m_int = constrain(m_int, -m_integral_limit, m_integral_limit);

        float i = m_int * m_ki;

        // D
        float d = 0;
        if (!m_first_sample) {
            float der = (err - m_prev_err) / dt;
            d         = der * m_kd;
        }
        m_prev_err     = err;
        m_first_sample = false;

        LOG_TRACE("PID: {}, {}, {}", p, i, d);

        return p + i + d;
    }

    auto reset() -> void {
        m_int          = 0;
        m_prev_err     = 0;
        m_first_sample = true;
    }

    auto get_target() noexcept -> const decltype(m_target) { return m_target; }
    auto set_target(float target) -> void { m_target = target; }

    auto set_integral_limit(float limit) -> void { m_integral_limit = limit; }
    auto get_integral_limit() -> float { return m_integral_limit; }

    auto get_paras() noexcept -> const std::tuple<float, float, float> {
        return std::make_tuple(m_kp, m_ki, m_kd);
    }

    auto set_paras(std::tuple<float, float, float> paras) -> void {
        auto [p, i, d] = paras;
        m_kp = p, m_ki = i, m_kd = d;
    }
};
