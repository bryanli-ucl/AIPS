#include <Arduino.h>

#include "kalman_filter.hpp"
#include "literals.hpp"
#include "peripherals.hpp"

using namespace ::literals;
using namespace ::peripherals;

Kalman_Filter pitch_angle_kf;
PID_Controller pitch_pid;
PID_Controller yaw_pid;
PID_Controller bot_vel_pid;

auto setup() -> void {
    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
    }

    { // board info
        LOG_SECTION("ARDUINO UNO R4 WIFI MASTER BOARS");
        LOG_INFO("sizeof(master_data): {}", sizeof(master_data));
        LOG_INFO("sizeof(slave_data): {}", sizeof(slave_data));
    }

    { // init peripherals
        LOG_SECTION("INITIALIZING PERIPHERALS");
        peripherals::begin();
    }

    { // init paras
        LOG_SECTION("INITIALIZING Parameters");

        LOG_INFO("Pitch PID");

        pitch_pid.reset();
        pitch_pid.set_paras({ 200.f, 50.f, 40.f });
        pitch_pid.set_target(0.0f);

        LOG_INFO("Motor Velocity PID");
        motor_l.reset();
        motor_l.set_paras({ 500.f, 50.f, 200.f });
        motor_l.set_target_avel(0rad_s);

        motor_l.reset();
        motor_r.set_paras({ 500.f, 50.f, 200.f });
        motor_r.set_target_avel(0rad_s);

        LOG_INFO("Yaw PID");
        yaw_pid.reset();
        yaw_pid.set_paras({ 1.f, 0.f, 0.f });
        yaw_pid.set_target(0);

        LOG_INFO("Bot Vel PID");
        bot_vel_pid.reset();
        bot_vel_pid.set_paras({ 1.f, 0.f, 0.f });
        bot_vel_pid.set_target(0);
    }

    LOG_SECTION("PROGRAM BEGIN");
}

auto get_pitch_rad(dura_t dt) -> float {

    float gyro_y  = imu.getRoll() * DEG_TO_RAD; // (rad/s)
    float accel_x = imu.getY();
    float accel_z = imu.getZ();

    float accel_ang = -atan2(accel_x, accel_z); // (rad)

    return pitch_angle_kf.update(gyro_y, accel_ang, dt);
}

auto get_yaw_rad(dura_t dt) -> float {
    float gyro_z = imu.getYaw() * DEG_TO_RAD; // (rad/s)

    static float s_yaw_angle = 0;
    s_yaw_angle += gyro_z * dt.v;
    return s_yaw_angle;
}

auto check_fall(float pitch_angle) -> void {
    constexpr auto FALL_THRESHOLD_RAD = 20 * RAD_TO_DEG;
    if (fabs(pitch_angle) > FALL_THRESHOLD_RAD) {
        while (true) {
            motoron.setAllSpeedsNow(0);
            delay(200);
        }
    }
}

auto task_5ms() -> void {
    LOG_TRACE("5 ms task");
    static constexpr dura_t dt = 5ms;

    imu.update();

    // bot vel pid
    float bot_vel = (motor_r.get_avel().v - motor_l.get_avel().v) * 0.5f;

    float target_pitch = bot_vel_pid.update(bot_vel, dt);
    target_pitch       = constrain(target_pitch, -5 * DEG_TO_RAD, 5 * DEG_TO_RAD);
    target_pitch       = 0;
    pitch_pid.set_target(target_pitch);


    // pitch pid
    float pitch_angle = get_pitch_rad(dt);
    float target_avel = pitch_pid.update(pitch_angle, dt);

    // safe check
    check_fall(pitch_angle);

    // yaw pid
    float yaw_angle = get_yaw_rad(dt);
    float yaw_corr  = yaw_pid.update(yaw_angle, dt);

    // mix velocity and rotation
    motor_l.set_target_avel(avel_t((target_avel - yaw_corr)));
    motor_r.set_target_avel(avel_t(-(target_avel + yaw_corr)));

    // update motor velocity pid
    motor_l.calc_velocity(dt);
    motor_l.update_power(dt);
    motor_r.calc_velocity(dt);
    motor_r.update_power(dt);
}

auto task_100ms() -> void {
    LOG_TRACE("100 ms task");

    static constexpr dura_t dt = 100ms;
}

auto task_500ms() -> void {
    LOG_TRACE("500 ms task");

    // LOG_INFO("knob cnt: {}", knob.get());
    LOG_INFO("Left Motor Status: {}, {}", motor_l.get_avel(), motor_l.get_count());
}

auto task_1s() -> void {
    LOG_TRACE("1s task");

    { // send data to slave

        // master_data.value1      = 1;
        // master_data.value2      = -2;
        // master_data.value3      = (micros() * 1us).v;
        // master_data.is_new_data = true;

        // Wire.beginTransmission(static_cast<uint8_t>(iic_addrs::SlaveMCU));
        // Wire.write((uint8_t*)&master_data, sizeof(master_data));
        // auto error = Wire.endTransmission();

        // if (error != 0)
        //     LOG_DEBUG("Transmission Error: {}", error);
    }
}

auto task_5s() -> void {
    LOG_TRACE("5s task");
}

auto loop() -> void {

    { // time stats
        static time_t last_5ms_timer   = -1;
        static time_t last_100ms_timer = -1;
        static time_t last_500ms_timer = -1;
        static time_t last_1s_timer    = -1;
        static time_t last_5s_timer    = -1;
        static time_t current_millis;

        current_millis = millis();

        if (current_millis - last_5ms_timer >= 5) {
            last_5ms_timer = current_millis;
            task_5ms();
        }
        if (current_millis - last_100ms_timer >= 100) {
            last_100ms_timer = current_millis;
            task_100ms();
        }
        if (current_millis - last_500ms_timer >= 500) {
            last_500ms_timer = current_millis;
            task_500ms();
        }
        if (current_millis - last_1s_timer >= 1000) {
            last_1s_timer = current_millis;
            task_1s();
        }
        if (current_millis - last_5s_timer >= 5000) {
            last_5s_timer = current_millis;
            task_5s();
        }
    }
}