#include "peripherals.hpp"
#include <Arduino.h>

#include "literals.hpp"
#include <exception>

using namespace ::literals;
using namespace ::peripherals;


auto setup() -> void {
    peripherals::begin();
}

auto task_1ms() -> void {
    LOG_TRACE("1 ms task");
}

auto task_10ms() -> void {
    LOG_TRACE("10 ms task");

    dura_t current_time = micros() * 1us;

    { // speed stats
        motor_l.calc_velocity(current_time);
        motor_l.update_power(current_time);
        motor_r.calc_velocity(current_time);
        motor_r.update_power(current_time);
    }
}

auto task_100ms() -> void {
    LOG_TRACE("100 ms task");
}

auto task_500ms() -> void {
    LOG_TRACE("500 ms task");
    // LOG_INFO("knob cnt: {}", knob.get());
    // LOG_INFO("Left Motor Status: {}, {}", enc_l.get_avel(), enc_l.get_count());
}

auto task_1s() -> void {
    LOG_TRACE("1s task");

    { // 发送数据到从机

        master_data.value1      = 1;
        master_data.value2      = -2;
        master_data.value3      = (micros() * 1us).v;
        master_data.is_new_data = true;

        Wire.beginTransmission(static_cast<uint8_t>(iic_addrs::SlaveMCU));
        Wire.write((uint8_t*)&master_data, sizeof(master_data));
        auto error = Wire.endTransmission();

        if (error != 0)
            LOG_DEBUG("Transmission Error: {}", error);
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