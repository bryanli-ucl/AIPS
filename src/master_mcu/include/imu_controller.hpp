#pragma once

#include "kalman_filter.hpp"
#include "literals.hpp"

#include <Arduino.h>
#include <Modulino.h>

using namespace ::literals;

class IMUController {
    public:
    struct State {
        float yaw        = 0.f;
        float pitch      = 0.f;
        float pitch_gyro = 0.f;
        float roll       = 0.f;
    };

    IMUController(ModulinoMovement& imu) : m_imu(imu) {}
    ~IMUController() = default;

    auto update(dura_t dt) -> const State& {

        m_imu.update();

        {                                             // yaw
            float gyro = m_imu.getYaw() * DEG_TO_RAD; // rad/s
            m_state.yaw += gyro * dt.v;
        }

        { // pitch
            float gyro = m_imu.getPitch() * DEG_TO_RAD;

            float ax    = m_imu.getY();
            float az    = m_imu.getZ();
            float accel = -atan2f(ax, az);

            float alpha = 0.90f;

            m_state.pitch = alpha * (m_state.pitch + gyro * dt.v) + (1 - alpha) * accel;

            m_state.pitch_gyro = gyro;
        }

        { // roll

            float gyro      = m_imu.getRoll() * DEG_TO_RAD; // rad/s
            float ay        = m_imu.getX();
            float az        = m_imu.getZ();
            float accel_ang = atan2f(ay, az); // rad
            m_state.roll    = m_roll_kf.update(gyro, accel_ang, dt);
        }
    }


    void reset() {
        m_pitch_kf = Kalman_Filter{};
        m_roll_kf  = Kalman_Filter{};
        m_state    = State{};
    }

    float get_yaw_rad() const { return m_state.yaw; }
    float get_yaw_deg() const { return m_state.yaw * RAD_TO_DEG; }

    float get_pitch_rad() const { return m_state.pitch; }
    float get_pitch_deg() const { return m_state.pitch * RAD_TO_DEG; }
    float get_pitch_gyro_rad() const { return m_state.pitch_gyro; }

    float get_roll_rad() const { return m_state.roll; }
    float get_roll_deg() const { return m_state.roll * RAD_TO_DEG; }

    private:
    ModulinoMovement& m_imu;

    Kalman_Filter m_pitch_kf;
    Kalman_Filter m_roll_kf;
    State m_state;
};
