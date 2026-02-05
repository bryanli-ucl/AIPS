// serial_logger.cpp
#include "dev/serial_logger.hpp"

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

} // namespace __details