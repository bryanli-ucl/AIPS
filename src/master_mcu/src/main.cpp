#include <Arduino.h>

#include "dev/peripherals.hpp"

#include "literals.hpp"
#include <exception>

using namespace ::literals;
using namespace ::peripherals;

auto setup() -> void {
    peripherals::begin();

    LOG_INFO("dis: {}, vel: {}, acc: {}", 1m, 1m_s, 1m_s / 1s);
    LOG_INFO("mass: {}, momtumum: {}, force: {}", 1kg, 1kg * 1m_s, 1N);
    LOG_INFO("eng: {}, frequency: {}", 1.6kJ, 80Hz);
}

auto task_1ms() -> void {
    LOG_TRACE("1 ms task");
}

auto task_10ms() -> void {
    LOG_TRACE("10 ms task");

    { // speed stats
        Motor::update_time();
        motor_l.calc_velocity();
        motor_r.calc_velocity();
    }
}

auto task_100ms() -> void {
    LOG_TRACE("100 ms task");
}

auto task_500ms() -> void {
    LOG_TRACE("500 ms task");
}

auto task_1s() -> void {
    LOG_TRACE("1s task");

    LOG_INFO("Motor ang_vel(rad/s): ({}, {})", motor_l.get_avel(), motor_r.get_avel());
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

        if (current_millis - last_1ms_timer >= 1000) {
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

    // LOG_INFO("{}", digitalRead(ENCODERL_A));

    // Serial.print(digitalRead(D2));

    // static uint32_t counter    = 0;
    // static uint32_t lastUpdate = 0;

    // if (millis() - lastUpdate >= 1000) {
    //     lastUpdate = millis();

    //     if (oled1362.is_enable()) {
    //         LOG_INFO("change disp message 1362");

    //         char line1[32];
    //         char line2[32];
    //         char line3[32];
    //         sprintf(line1, "Count: %lu", counter++);
    //         sprintf(line2, "Time: %lu s", millis() / 1000);
    //         sprintf(line3, "Menu: %s", "Device DISP");

    //         oled1362.disp_status(line3, line1, line2);
    //     }

    //     if (oled1306.is_enable()) {
    //         LOG_INFO("change disp message 1306");
    //         oled1306.disp_status("Bryan", "hello", "world");
    //     }
    // }
}