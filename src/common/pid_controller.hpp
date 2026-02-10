#pragma once

#include <stdint.h>

#include "literals.hpp"

namespace ctrl {

using namespace ::literals;

class pid_controller {
    private:
    double m_target;

    double m_kp;
    double m_ki;
    double m_kd;

    double m_int;
    double m_prev_err;

    dura_t m_prev_time;

    bool m_first_sample;

    public:
    pid_controller() noexcept
    : m_target(.0),
      m_kp(.0), m_ki(.0), m_kd(.0),
      m_int(.0),
      m_prev_err(.0),
      m_prev_time(0s),
      m_first_sample(true) {
    }

    pid_controller(double kp, double ki, double kd) noexcept
    : pid_controller() {
        m_kp = kp;
        m_ki = ki;
        m_kd = kd;
    }

    ~pid_controller() noexcept = default;

    auto update(float val, dura_t now_time) -> float {
        float dt = now_time.v - m_prev_time.v;
        if (m_first_sample) dt = 0;

        float err = m_target - val;

        float p = err * m_kp;

        m_int += err * dt;
        float i = m_int * m_ki;

        float d = 0;
        if (!m_first_sample) {
            float der = -(err - m_prev_err) / dt;
            d         = der * m_kd;
        }

        m_prev_err     = err;
        m_prev_time    = now_time;
        m_first_sample = false;

        float output = p + i + d;
        return output;
    }

    auto reset() -> void {
        m_int          = 0;
        m_prev_err     = 0;
        m_first_sample = true;
    }

    auto get_target() noexcept -> const decltype(m_target) { return m_target; }
    auto set_target(float target) -> void { m_target = target; }

    auto get_paras(std::tuple<float, float, float> paras) noexcept -> const std::tuple<float, float, float> {
        return std::make_tuple(m_kp, m_ki, m_kd);
    }

    auto set_paras(std::tuple<float, float, float> paras) noexcept -> void {
        auto [p, i, d] = paras;
        m_kp = p, m_ki = i, m_kd = d;
    }

    auto get_kp() noexcept -> const decltype(m_kp) { return m_kp; }
    auto set_kp(float kp) -> void { m_kp = kp; }

    auto get_ki() noexcept -> const decltype(m_ki) { return m_ki; }
    auto set_ki(float ki) -> void { m_ki = ki; }

    auto get_kd() noexcept -> const decltype(m_kd) { return m_kd; }
    auto set_kd(float kd) -> void { m_kd = kd; }
};

} // namespace ctrl
