#include <Arduino.h>

#include "firmware.hpp"
#include "literals.hpp"

#include <exception>

using namespace ::literals;

volatile int64_t motorL_cnt = 0;
volatile int64_t motorR_cnt = 0;

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
}
auto task_500ms() -> void {
    LOG_TRACE("500 ms task");
}
auto task_1s() -> void {
    LOG_TRACE("1s task");
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