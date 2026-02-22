#include "literals.hpp"
#include "peripherals.hpp"
#include <Arduino.h>
#include <WiFiS3.h>
#include <Wire.h>

using namespace ::peripherals;
using namespace ::literals;

void setup() {
    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
    }

    { // board info
        LOG_SECTION("ARDUINO UNO R4 WIFI SLAVE BOARS");
        LOG_INFO("sizeof(master_data): {}", sizeof(master_to_slave_iic_data_t));
        LOG_INFO("sizeof(slave_data): {}", sizeof(slave_to_master_iic_data_t));
    }

    { // init peripherals
        LOG_SECTION("INITIALIZING PERIPHERALS");
        peripherals::begin();
    }

    LOG_SECTION("PROGRAM BEGIN");
}

int8_t process_udp_data(int pack_len) {

    static uint8_t buf[256] = {};

    if (pack_len == 0)
        return -1;

    LOG_TRACE("Received From {}:{}, Packet Size: {}", udp.remoteIP(), udp.remotePort(), pack_len);

    int len = udp.read(buf, sizeof(buf));

    if (len != sizeof(PC_to_robot_wifi_data_t)) {
        LOG_WARN("Error Length UDP pack");
        return -2;
    }

    PC_to_robot_wifi_data_t* data = reinterpret_cast<PC_to_robot_wifi_data_t*>(&buf);

    LOG_DEBUG("Received Contents: {}, {}", data->vel_x, data->vel_y);

    udp.beginPacket(udp.remoteIP(), udp.remotePort());
    udp.write("OK");
    udp.endPacket();

    return 0;
}

int8_t process_iic_data() {
    auto& data = iic_commu::master_data;
    if (!data.is_new_data) {
        return -1;
    }

    data.is_new_data = false;

    LOG_INFO("DATA.value1: {}", data.value1);
    LOG_INFO("DATA.value2: {}", data.value2);
    LOG_INFO("DATA.value3: {}", data.value3);
}

void loop() {
    process_udp_data(udp.parsePacket());
    process_iic_data();
}