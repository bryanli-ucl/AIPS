#include <Arduino.h>
#include <cstring>

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
float g_pitch_target_rad = 0.11f;

auto setup() -> void {

    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
    }

    { // board info
        LOG_SECTION("ARDUINO UNO R4 WIFI MASTER BOARS");
        LOG_INFO("sizeof(master_data): {}", sizeof(master_data));
        LOG_INFO("sizeof(slave2master_data): {}", sizeof(slave2master_data));
    }

    { // init peripherals
        LOG_SECTION("INITIALIZING PERIPHERALS");
        peripherals::begin();
    }

    { // init paras
        LOG_SECTION("INITIALIZING Parameters");

        LOG_INFO("Pitch PID");

        pitch_pid.reset();
        pitch_pid.set_paras({ 100.f, 0.0f, 7.5f });
        pitch_pid.set_target(g_pitch_target_rad);
        pitch_pid.set_integral_limit(300);

        LOG_INFO("Motor Velocity PID");
        motor_l.reset();
        motor_l.set_paras({ 30.f, 20.f, 0.f });
        motor_l.set_integral_limit(200.f);
        motor_l.set_target_avel(0rad_s);
        motor_l.set_power_constrain(400);
        motor_l.set_dead_zone(50);

        motor_r.reset();
        motor_r.set_paras({ 30.f, 20.f, 0.f });
        motor_r.set_integral_limit(200.f);
        motor_r.set_target_avel(0rad_s);
        motor_r.set_power_constrain(400);
        motor_r.set_dead_zone(50);

        LOG_INFO("Yaw PID");
        yaw_pid.reset();
        yaw_pid.set_paras({ 10.f, 20.f, 0.f });
        yaw_pid.set_target(0);

        LOG_INFO("Bot Vel PID");
        bot_vel_pid.reset();
        bot_vel_pid.set_paras({ 1.f, 0.f, 0.f });
        bot_vel_pid.set_target(0);
    }

    LOG_SECTION("PROGRAM BEGIN");

    { // Run Once
        if constexpr (false) {


            while (true) {
                noInterrupts();
                delay(100);
            }
        }
    }

    { // Scheduler & Tasks

        scheduler.add(-1, []() { // Fall Check
            static constexpr dura_t dt        = 200ms;
            constexpr auto FALL_THRESHOLD_RAD = 30 * DEG_TO_RAD;
            if (imu_ctrl.get_pitch_rad() > FALL_THRESHOLD_RAD) {
                while (true) {
                    motoron.setAllSpeedsNow(0);
                    // asm volatile("halt");
                    delay(300);
                    LOG_FATAL("Fall Down Halted");
                }
            }
        },
        "Fall Check");

        scheduler.add(80, []() { // Board Communication
            static constexpr dura_t dt = 80ms;

            // master_data.value1      = 1;
            // master_data.value2      = -2;
            // master_data.value3      = dt.v;
            // master_data.is_new_data = true;
            // Wire.beginTransmission(static_cast<uint8_t>(iic_addrs::SlaveMCU));
            // Wire.write((uint8_t*)&master_data, sizeof(master_data));
            // auto error = Wire.endTransmission();

            // if (error != 0)
            //     LOG_DEBUG("Transmission Error: {}", error);

            static slave_to_master_iic_data_t data{};

            int len = Wire1.requestFrom(iic_addrs::SlaveMCU, sizeof(data));

            if (len != sizeof(data)) {
                while (Wire1.available()) Wire1.read();
                return;
            }

            {
                uint8_t* ptr = reinterpret_cast<uint8_t*>(&data);
                while (Wire1.available()) {
                    *ptr = Wire1.read();
                    ptr++;
                }
            }

            LOG_INFO("From Slave: bot vel: {}, yaw vel: {}", data.target_vel, data.target_yaw);

        },
        "Board Communication");

        scheduler.add(-1, []() { // Print CPU Usage
            scheduler.print_cpu_usage();
        },
        "Print CPU Usage");

        scheduler.add(500, []() { // Print Stats
            static constexpr dura_t dt = 300ms;
            // LOG_INFO("Left Motor Status: pwr:{}, avel:{}, pos:{}", motor_l.get_power(), motor_l.get_avel(), motor_l.get_count());
            // LOG_INFO("Right Motor Status: pwr:{}, avel:{}, pos:{}", motor_r.get_power(), motor_r.get_avel(), motor_r.get_count());
            // LOG_INFO("State: Roll{}, Pitch{}, Yaw{}", imu_ctrl.get_roll_deg(), imu_ctrl.get_pitch_deg(), imu_ctrl.get_yaw_deg());
            LOG_INFO("{} {} {} {} {}", motor_r.get_avel(), motor_r.get_power(), motor_l.get_avel(), motor_l.get_power(), imu_ctrl.get_pitch_deg());
            LOG_INFO("Knob: {}", knob.get());
        },
        "Print Stats");

        scheduler.add(200, []() { // Process UDP for Pitch PID tuning
            static uint8_t buf[128] = {};

            const int pack_len = udp.parsePacket();
            if (pack_len <= 0) return;

            const int len = udp.read(buf, sizeof(buf) - 1);
            if (len <= 0) return;

            float target = g_pitch_target_rad;
            float kp, ki, kd;
            std::tie(kp, ki, kd) = pitch_pid.get_paras();

            bool parsed = false;

            if (len == static_cast<int>(sizeof(float) * 4)) {
                float vals[4];
                memcpy(vals, buf, sizeof(vals));
                target = vals[0];
                kp     = vals[1];
                ki     = vals[2];
                kd     = vals[3];
                parsed = true;
            }

            if (!parsed) {
                LOG_WARN("Unknown UDP payload format, len={}, {}", len, reinterpret_cast<const char*>(buf));
                return;
            }

            g_pitch_target_rad = target;
            pitch_pid.set_paras({ kp, ki, kd });
            pitch_pid.set_target(g_pitch_target_rad);
            pitch_pid.reset();

            LOG_INFO("Pitch PID updated by UDP: target={}, kp={}, ki={}, kd={}",
            g_pitch_target_rad, kp, ki, kd);

            char ack[96];
            snprintf(ack, sizeof(ack), "OK target=%.5f kp=%.5f ki=%.5f kd=%.5f\n",
            g_pitch_target_rad, kp, ki, kd);
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write(reinterpret_cast<const uint8_t*>(ack), strlen(ack));
            udp.endPacket();
        },
        "Process UDP Pitch PID");

        scheduler.add(7, []() { // Update IMU
            static constexpr dura_t dt = 20ms;
            imu_ctrl.update(dt);
        },
        "Update IMU");

        scheduler.add(5, []() { // Main PID Controller
            static constexpr dura_t dt = 20ms;

            // bot vel pid
            float bot_vel = (motor_r.get_avel().v - motor_l.get_avel().v) * 0.5f;

            float target_pitch = bot_vel_pid.update(bot_vel, dt);
            target_pitch       = constrain(target_pitch, -5 * DEG_TO_RAD, 5 * DEG_TO_RAD);
            target_pitch       = g_pitch_target_rad;
            pitch_pid.set_target(target_pitch);

            // pitch pid
            float pitch_angle = imu_ctrl.get_pitch_rad();
            float target_avel = pitch_pid.update(-pitch_angle, dt);
            LOG_TRACE("Target Vel: {}, pitch_angle: {}", target_avel, pitch_angle);

            // yaw pid
            float yaw_angle = imu_ctrl.get_yaw_rad();
            float yaw_corr  = yaw_pid.update(yaw_angle, dt);

            // mix velocity and rotation
            yaw_corr = 0;
            motor_l.set_target_avel(avel_t((target_avel - atanf(yaw_corr) * (1 / TWO_PI))));
            motor_r.set_target_avel(-avel_t((target_avel + atanf(yaw_corr) * (1 / TWO_PI))));

            // motor_l.update_power_force(-10 * ((target_avel - atanf(yaw_corr) * (1 / TWO_PI))));
            // motor_r.update_power_force(-10 * ((target_avel + atanf(yaw_corr) * (1 / TWO_PI))));

        },
        "Main PID");

        scheduler.add(5, []() { // update motor velocity pid
            static constexpr dura_t dt = 20ms;

            // motor_l.set_target_avel(5rad_s);
            // motor_r.set_target_avel(-5rad_s);

            motor_l.calc_velocity(dt);
            motor_l.update_power(dt);
            motor_r.calc_velocity(dt);
            motor_r.update_power(dt);
        },
        "Update Motor");

        scheduler.reset();
    }
}

auto loop() -> void {
    scheduler.tick(millis());
}
