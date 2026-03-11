#include "iic_commu.hpp"

#include <Arduino.h>
#include <Wire.h>

master_to_slave_iic_data_t iic_commu::master_data;
slave_to_master_iic_data_t iic_commu::slave2master_data;

auto iic_commu::rec_event_callback(int len) -> void {
    if (len != sizeof(decltype(master_data))) {
        while (Wire1.available()) Wire1.read();
        return;
    }
    Wire1.readBytes((uint8_t*)&master_data, len);
}

auto iic_commu::req_event_callback() -> void {
    Wire1.write((uint8_t*)&slave2master_data, sizeof(decltype(slave2master_data)));
}

auto iic_commu::begin() -> void {
    Wire1.onReceive(iic_commu::rec_event_callback);
    Wire1.onRequest(iic_commu::req_event_callback);
}
