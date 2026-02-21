#include <Arduino.h>
#include <Wire.h>

#include "literals.hpp"
#include "peripherals.hpp"

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
char ReplyBuffer[] = "acknowledged\n";
char packetBuffer[256];
void loop() {
    int packetSize = udp.parsePacket();
    if (packetSize) {
        Serial.print("Received packet of size ");
        Serial.println(packetSize);
        Serial.print("From ");
        IPAddress remoteIp = udp.remoteIP();
        Serial.print(remoteIp);
        Serial.print(", port ");
        Serial.println(udp.remotePort());

        // read the packet into packetBuffer
        int len = udp.read(packetBuffer, 255);
        if (len > 0) {
            packetBuffer[len] = 0;
        }
        Serial.println("Contents:");
        Serial.println(packetBuffer);

        // send a reply, to the IP address and port that sent us the packet we received
        udp.beginPacket(udp.remoteIP(), udp.remotePort());
        udp.write(ReplyBuffer);
        udp.endPacket();
    }
    auto& data = iic_commu::master_data;

    if (data.is_new_data) {
        data.is_new_data = false;

        LOG_INFO("DATA.value1: {}", data.value1);
        LOG_INFO("DATA.value2: {}", data.value2);
        LOG_INFO("DATA.value3: {}", data.value3);
    }
}