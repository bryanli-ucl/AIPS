#include <Arduino.h>

#include "imu_controller.hpp"
#include "literals.hpp"
#include "peripherals.hpp"
#include "task_scheduler.hpp"

using namespace ::literals;
using namespace ::peripherals;

IMUController imu_ctrl{ imu };
PID_Controller pitch_pid;
PID_Controller yaw_pid;
PID_Controller bot_vel_pid;
TaskScheduler scheduler;

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
        pitch_pid.set_paras({ 20.f, 2.f, 5.f });
        pitch_pid.set_target(0.0f);

        LOG_INFO("Motor Velocity PID");
        motor_l.reset();
        motor_l.set_paras({ 15.f, 1.f, 1.f });
        motor_l.set_integral_limit(50.f);
        motor_l.set_target_avel(0rad_s);

        motor_r.reset();
        motor_r.set_paras({ 15.f, 1.f, 1.f });
        motor_r.set_integral_limit(50.f);
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

    { // Scheduler

        scheduler.add(200, []() { // Fall Check
            static constexpr dura_t dt        = 200ms;
            constexpr auto FALL_THRESHOLD_RAD = 20 * RAD_TO_DEG;
            if (imu_ctrl.get_pitch_rad() > FALL_THRESHOLD_RAD) {
                while (true) {
                    motoron.setAllSpeedsNow(0);
                    // asm volatile("halt");
                    delay(300);
                    LOG_FATAL("Fall Down Halted");
                }
            }
        });

        scheduler.add(200, []() { // Board Communication
            static constexpr dura_t dt = 200ms;

            master_data.value1      = 1;
            master_data.value2      = -2;
            master_data.value3      = dt.v;
            master_data.is_new_data = true;

            Wire.beginTransmission(static_cast<uint8_t>(iic_addrs::SlaveMCU));
            Wire.write((uint8_t*)&master_data, sizeof(master_data));
            auto error = Wire.endTransmission();

            if (error != 0)
                LOG_DEBUG("Transmission Error: {}", error);
        });

        scheduler.add(300, []() { // Print Stats
            static constexpr dura_t dt = 300ms;
            LOG_INFO("Left Motor Status: pwr:{}, avel:{}, pos:{}", motor_l.get_power(), motor_l.get_avel(), motor_l.get_count());
            LOG_INFO("Right Motor Status: pwr:{}, avel:{}, pos:{}", motor_r.get_power(), motor_r.get_avel(), motor_r.get_count());
            LOG_INFO("State: Roll{}, Pitch{}, Yaw{}", imu_ctrl.get_roll_deg(), imu_ctrl.get_pitch_deg(), imu_ctrl.get_yaw_deg());
        });

        scheduler.add(5, []() { // Main PID Controller
            static constexpr dura_t dt = 5ms;

            imu_ctrl.update(dt);

            // bot vel pid
            float bot_vel = (motor_r.get_avel().v - motor_l.get_avel().v) * 0.5f;

            float target_pitch = bot_vel_pid.update(bot_vel, dt);
            target_pitch       = constrain(target_pitch, -5 * DEG_TO_RAD, 5 * DEG_TO_RAD);
            target_pitch       = 0;
            pitch_pid.set_target(target_pitch);

            // pitch pid
            float pitch_angle = imu_ctrl.get_pitch_rad();
            float target_avel = pitch_pid.update(pitch_angle, dt);
            LOG_TRACE("Target Vel: {}, pitch_angle: {}", target_avel, pitch_angle);

            // yaw pid
            float yaw_angle = imu_ctrl.get_yaw_rad();
            float yaw_corr  = yaw_pid.update(yaw_angle, dt);

            // mix velocity and rotation
            yaw_corr = 0;
            motor_l.set_target_avel(avel_t((target_avel - atanf(yaw_corr) * (1 / TWO_PI))));
            motor_r.set_target_avel(-avel_t((target_avel + atanf(yaw_corr) * (1 / TWO_PI))));

            // update motor velocity pid
            motor_l.calc_velocity(dt);
            motor_l.update_power(dt);
            motor_r.calc_velocity(dt);
            motor_r.update_power(dt);
        });
    }
}

auto loop() -> void {
    scheduler.tick(millis());
}