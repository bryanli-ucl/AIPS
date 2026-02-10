// serial_logger.cpp
#include "serial_logger.hpp"

#include <Arduino.h>

namespace __details {

void begin() {
    if constexpr (LOG_LEVEL == LOG_LEVEL_OFF)
        return;
    Serial.begin(LOG_BAUDRATE);

    unsigned long start = millis();
    while (!Serial && (millis() - start < 1000)) {
        delay(10);
    }
}

void print_header(f_ptr level, const f_ptr location) {
    if constexpr (LOG_SHOW_LEVEL) {
        Serial.print(level);
    }
    if constexpr (LOG_SHOW_LOCATION) {
        Serial.print(location);
    }
}

void print_section(f_ptr str, const uint16_t len) {
    for (int i = 0; i < len; i++)
        Serial.print('=');
    Serial.println();

    for (int i = 0; i < 10; i++)
        Serial.print('=');
    Serial.print(' ');
    Serial.println(str);

    for (int i = 0; i < len; i++)
        Serial.print('=');
    Serial.println();
}

} // namespace __details