#include "dev/iic_commu.hpp"

#include <Arduino.h>
#include <Wire.h>

master_iic_data_t iic_commu::master_data;
slave_iic_data_t iic_commu::slave_data;

auto iic_commu::rec_event_callback(int len) -> void {
    digitalWrite(LED_BUILTIN, HIGH);
    if (len != sizeof(decltype(master_data))) {
        while (Wire.available()) Wire.read();
        digitalWrite(LED_BUILTIN, LOW);
        return;
    }
    Wire.readBytes((uint8_t*)&master_data, len);
    digitalWrite(LED_BUILTIN, LOW);
}

auto iic_commu::req_event_callback() -> void {
    Wire.write((uint8_t*)&slave_data, sizeof(decltype(slave_data)));
}

auto iic_commu::begin() -> void {
    pinMode(LED_BUILTIN, OUTPUT);
    Wire.onReceive(rec_event_callback);
    Wire.onRequest(req_event_callback);
}
