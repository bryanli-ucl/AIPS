#include <Arduino.h>

#include "firmware.hpp"
#include "literals.hpp"

#include <exception>

using namespace ::literals;

volatile int64_t motorL_cnt = 0;
volatile int64_t motorR_cnt = 0;

rot_t rad_l = 0Hz;
rot_t rad_r = 0Hz;

void motorL_isr() {
    if (digitalRead(ENCODERL_A) == digitalRead(ENCODERL_B))
        motorL_cnt++;
    else
        motorL_cnt--;
}

void motorR_isr() {
    if (digitalRead(ENCODERR_A) == digitalRead(ENCODERR_B))
        motorR_cnt++;
    else
        motorR_cnt--;
}

auto setup() -> void {

    firmware_begin();

    pinMode(ENCODERL_A, INPUT_PULLUP);
    pinMode(ENCODERL_B, INPUT_PULLUP);
    pinMode(ENCODERR_A, INPUT_PULLUP);
    pinMode(ENCODERR_B, INPUT_PULLUP);

    noInterrupts();
    attachInterrupt(digitalPinToInterrupt(ENCODERL_A), motorL_isr, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ENCODERR_A), motorR_isr, CHANGE);
    interrupts();
}

auto task_100ms() -> void {
    LOG_TRACE("100 ms task");

    static time_t prev_time  = 0;
    static dura_t delta_time = 0s;
    delta_time               = dura_t((micros() - prev_time) / 1000'000.0);
    prev_time                = micros();

    { // speed stats

        constexpr float GEAR_RATIO = (22.0 / 12.0) * (22.0 / 10.0) * (24.0 / 10.0);
        constexpr float RAW_CPR    = 48.0;
        constexpr float CPR_REV    = 1 / (GEAR_RATIO * RAW_CPR);

        int64_t encl = motorL_cnt;
        int64_t encr = motorR_cnt;

        static int64_t prev_encl = encl;
        static int64_t prev_encr = encr;

        float deltal = encl - prev_encl;
        float deltar = encr - prev_encr;

        rad_l = deltal * CPR_REV * TWO_PI / delta_time;
        rad_r = deltar * CPR_REV * TWO_PI / delta_time;

        LOG_TRACE("{} * {}, delta time: {}s", deltal / CPR, (2.0 * PI / delta_time).v, delta_time.v);

        prev_encl = encl;
        prev_encr = encr;
    }
}

auto task_500ms() -> void {
    LOG_TRACE("500 ms task");
}

auto task_1s() -> void {
    LOG_TRACE("1s task");

    LOG_INFO("Motor L angular velocity: {}rad/s, cnt: {}", rad_l.v, motorL_cnt);
}

auto task_5s() -> void {
    LOG_TRACE("5s task");
}


auto loop() -> void {

    { // time stats
        static time_t last_100ms_timer = -114514;
        static time_t last_500ms_timer = -114514;
        static time_t last_1s_timer    = -114514;
        static time_t last_5s_timer    = -114514;
        static time_t current_time;

        current_time = millis();

        if (current_time - last_100ms_timer >= 100) {
            last_100ms_timer = current_time;
            task_100ms();
        }
        if (current_time - last_500ms_timer >= 500) {
            last_500ms_timer = current_time;
            task_500ms();
        }
        if (current_time - last_1s_timer >= 1000) {
            last_1s_timer = current_time;
            task_1s();
        }
        if (current_time - last_5s_timer >= 5000) {
            last_5s_timer = current_time;
            task_5s();
        }
    }

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