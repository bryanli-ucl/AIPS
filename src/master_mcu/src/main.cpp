#include <Arduino.h>

#include "kalman_filter.hpp"
#include "literals.hpp"
#include "peripherals.hpp"

using namespace ::literals;
using namespace ::peripherals;

Kalman_Filter angle_pitch_kf;
PID_Controller angle_pid;

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

        LOG_INFO("Angle Ring PID");

        angle_pid.reset();
<<<<<<< HEAD
        angle_pid.set_paras({ 700.f, 1500.f, 40.f });
        angle_pid.set_target(0.0);
=======
        angle_pid.set_paras({ 200.f, 50.f, 40.f });
        angle_pid.set_target(0.0f);
>>>>>>> 6fe8b202422152bd1f809bba49aaae0bb34a242d

        LOG_INFO("Velocity Ring PID");
        motor_l.set_target_avel(20rad_s);
        motor_l.set_paras({ 500.f, 50.f, 200.f });

        motor_r.set_target_avel(0rad_s);
        motor_r.set_paras({ 500.f, 50.f, 200.f });
    }

    LOG_SECTION("PROGRAM BEGIN");
}

auto task_1ms() -> void {
    LOG_TRACE("1 ms task");
}

auto task_10ms() -> void {
    LOG_TRACE("10 ms task");

    dura_t dt = 10ms;

    { // pid angle ring
        imu.update();

        float gyro_y  = imu.getRoll() * DEG_TO_RAD;
        float accel_x = imu.getY();
        float accel_z = imu.getZ();

        float accel_mag = sqrt(accel_x * accel_x + accel_z * accel_z);
        float accel_ang = -atan2(accel_x, accel_z);

        float pitch_angle = angle_pitch_kf.update(gyro_y, accel_ang, dt);

        if (fabs(pitch_angle) > 20 * DEG_TO_RAD) {
            while(1){
                motoron.setAllSpeedsNow(0);
                delay(100);
            }
        }

        float motor_speed = angle_pid.update(pitch_angle, dt);

        // LOG_DEBUG("angle: {}, motor_speed: {}", pitch_angle * RAD_TO_DEG, motor_speed);

        // motor_l.set_target_avel(avel_t(motor_speed));
        motor_l.set_target_avel(10rad_s);
        motor_r.set_target_avel(10rad_s);
        // motor_r.set_target_avel(avel_t(-motor_speed));
    }

    { // pid speed ring
        motor_l.calc_velocity(dt);
        motor_l.update_power(dt);
        motor_r.calc_velocity(dt);
        motor_r.update_power(dt);
    }
}

auto task_100ms() -> void {
    LOG_TRACE("100 ms task");
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
        static time_t last_1ms_timer   = -1;
        static time_t last_10ms_timer  = -1;
        static time_t last_100ms_timer = -1;
        static time_t last_500ms_timer = -1;
        static time_t last_1s_timer    = -1;
        static time_t last_5s_timer    = -1;
        static time_t current_millis;
        static time_t current_micro;

        current_millis = millis();
        current_micro  = micros();

        if (current_micro - last_1ms_timer >= 1000) {
            last_1ms_timer = current_micro;
            task_1ms();
        }
        if (current_millis - last_10ms_timer >= 10) {
            last_10ms_timer = current_millis;
            task_10ms();
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