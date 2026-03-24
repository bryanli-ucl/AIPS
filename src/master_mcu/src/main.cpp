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

struct {
    float pitch_target_eq_rad = -0.005f;
} constant;

static slave_to_master_iic_data_t s2m_data{};

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
        pitch_pid.set_paras({ 210.f, 0.0f, 3.f });
        pitch_pid.set_target(constant.pitch_target_eq_rad);
        pitch_pid.set_integral_limit(300);

        LOG_INFO("Motor Velocity PID");
        motor_l.reset();
        motor_l.set_paras({ 30.f, 20.f, 0.f });
        motor_l.set_integral_limit(200.f);
        motor_l.set_target_avel(0rad_s);
        motor_l.set_power_constrain(400);
        motor_l.set_dead_zone(20);

        motor_r.reset();
        motor_r.set_paras({ 30.f, 20.f, 0.f });
        motor_r.set_integral_limit(200.f);
        motor_r.set_target_avel(0rad_s);
        motor_r.set_power_constrain(400);
        motor_r.set_dead_zone(20);

        LOG_INFO("Yaw PID");
        yaw_pid.reset();
        yaw_pid.set_paras({ 10.f, 20.f, 0.f });
        yaw_pid.set_target(0);

        LOG_INFO("Bot Vel PID");
        bot_vel_pid.reset();
        bot_vel_pid.set_paras({ .00002f, 0.00000001f, 0.000001f });
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
            if (fabs(imu_ctrl.get_pitch_rad()) > FALL_THRESHOLD_RAD) {
                while (true) {
                    motoron.setAllSpeedsNow(0);
                    // asm volatile("halt");
                    delay(300);
                    LOG_FATAL("Fall Down Halted");
                }
            }
        },
        "Fall Check");

        scheduler.add(-1, []() { // Board Communication
            static constexpr dura_t dt = 80ms;

            int len = Wire1.requestFrom(iic_addrs::SlaveMCU, sizeof(s2m_data));

            if (len != sizeof(s2m_data)) {
                while (Wire1.available()) Wire1.read();
                return;
            }

            {
                uint8_t* ptr = reinterpret_cast<uint8_t*>(&s2m_data);
                while (Wire1.available()) {
                    *ptr = Wire1.read();
                    ptr++;
                }
            }

            LOG_INFO("From Slave: bot vel: {}, yaw vel: {}", s2m_data.target_vel, s2m_data.target_yaw);

        },
        "Board Communication");

        scheduler.add(5000, []() { // Print CPU Usage
            scheduler.print_cpu_usage();
        },
        "Print CPU Usage");

        scheduler.add(-1, []() { // Print Stats
            static constexpr dura_t dt = 300ms;
            LOG_INFO("Left Motor Status: pwr:{}, avel:{}, pos:{}", motor_l.get_power(), motor_l.get_avel(), motor_l.get_count());
            LOG_INFO("Right Motor Status: pwr:{}, avel:{}, pos:{}", motor_r.get_power(), motor_r.get_avel(), motor_r.get_count());
            LOG_INFO("State: Roll{}, Pitch{}, Yaw{}", imu_ctrl.get_roll_deg(), imu_ctrl.get_pitch_deg(), imu_ctrl.get_yaw_deg());
        },
        "Print Stats");

        scheduler.add(100, []() { // UDP
            struct udp_pid_packet_t {
                uint8_t pid_id;
                float target;
                float kp;
                float ki;
                float kd;
            } __attribute__((__packed__));

            static uint8_t buf[128];

            int pack_len = udp.parsePacket();
            if (pack_len <= 0) return;

            int len = udp.read(buf, sizeof(buf));

            if (len != sizeof(udp_pid_packet_t)) {
                LOG_WARN("UDP packet size error {}", len);
                return;
            }
            auto* pkt = reinterpret_cast<udp_pid_packet_t*>(buf);

            float target = pkt->target;
            float kp     = pkt->kp;
            float ki     = pkt->ki;
            float kd     = pkt->kd;

            auto update_pid = [&](PID_Controller& pid) {
                pid.set_paras({ kp, ki, kd });
                pid.set_target(target);
                pid.reset();
            };

            switch (pkt->pid_id) {
            case 0:
                constant.pitch_target_eq_rad = target;
                update_pid(pitch_pid);
                break;

            case 1:
                update_pid(yaw_pid);
                break;

            case 2:
                update_pid(bot_vel_pid);
                break;

            case 3:
                motor_l.set_paras({ kp, ki, kd });
                break;

            case 4:
                motor_r.set_paras({ kp, ki, kd });
                break;
            }

            LOG_INFO(
            "PID {} updated target={} kp={} ki={} kd={}",
            pkt->pid_id,
            pkt->target,
            pkt->kp,
            pkt->ki,
            pkt->kd);

            char ack[64];

            snprintf(
            ack,
            sizeof(ack),
            "OK pid=%d kp=%.2f ki=%.2f kd=%.2f",
            pkt->pid_id,
            pkt->kp,
            pkt->ki,
            pkt->kd);

            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write((uint8_t*)ack, strlen(ack));
            udp.endPacket();
        },
        "UDP PID Tuning");

        scheduler.add(5, []() { // Main PID Controller
            static constexpr dura_t dt = 5ms;

            imu_ctrl.update(dt);
            // bot vel pid
            float bot_vel = (motor_r.get_avel().v + (-motor_l.get_avel().v)) * 0.5f;

            // float target_pitch = bot_vel_pid.update(bot_vel, dt);
            float target_pitch = bot_vel_pid.update(bot_vel, dt);
            target_pitch       = constrain(target_pitch, -5 * DEG_TO_RAD, 5 * DEG_TO_RAD);
            // target_pitch       = 0;
            pitch_pid.set_target(target_pitch + constant.pitch_target_eq_rad);

            // pitch pid
            // float target_avel = pitch_pid.update(-pitch_angle, dt, imu_ctrl.get_pitch_gyro_rad());
            float target_avel;
            {
                float pitch_angle = imu_ctrl.get_pitch_rad();
                float pitch_gyro  = imu_ctrl.get_pitch_gyro_rad();
                auto [kp, ki, kd] = pitch_pid.get_paras();
                auto err          = pitch_pid.get_target() - (pitch_angle);
                auto err_gyro     = pitch_pid.get_target() - (pitch_gyro);
                target_avel       = kp * err - kd * err_gyro;
            }
            LOG_TRACE("Target Vel: {}, pitch_angle: {}", target_avel, imu_ctrl.get_pitch_rad());

            // yaw pid
            float yaw_angle = imu_ctrl.get_yaw_rad();
            float yaw_corr  = yaw_pid.update(yaw_angle, dt);

            // mix velocity and rotation
            yaw_corr = 0;
            // motor_l.set_target_avel(avel_t((target_avel - atanf(yaw_corr) * (1 / TWO_PI))));
            // motor_r.set_target_avel(-avel_t((target_avel + atanf(yaw_corr) * (1 / TWO_PI))));

            motor_l.update_power_force(30 * ((target_avel - yaw_corr)));
            motor_r.update_power_force(-30 * ((target_avel + yaw_corr)));

        },
        "Main PID");

        scheduler.add(50, []() { // update motor velocity pid
            static constexpr dura_t dt = 50ms;

            // motor_l.set_target_avel(5rad_s);
            // motor_r.set_target_avel(-5rad_s);

            motor_l.calc_velocity(dt);
            // motor_l.update_power(dt);
            motor_r.calc_velocity(dt);
            // motor_r.update_power(dt);
        },
        "Update Motor");

        scheduler.reset();
    }
}

auto loop() -> void {
    scheduler.tick(millis());
}
