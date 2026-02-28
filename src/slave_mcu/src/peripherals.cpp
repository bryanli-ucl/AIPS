// peripherals.cpp
#include "peripherals.hpp"

#include <WiFiS3.h>

namespace peripherals {

dev_oled1362 oled1362{ OLED_CS, OLED_DC, OLED_RST };                    // SPI
dev_oled1306 oled1306{};                                                // IIC
WifiCommu udp_comm{ NETWORK_SSID, NETWORK_PASSWORD, NETWORK_UDP_PORT }; // WiFi
QTRSensors qtr{};                                                       // IR Sensors

auto begin() -> void {

    { // SPI
        LOG_INFO_START("Initializing SPI");
        if (initializing_list.SPI) {
            delay(10);
            SPI.begin();
            delay(300);
            LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // iic
        LOG_INFO_START("Initializing IIC");
        if (initializing_list.IIC) {
            // iic pinmode (pullup resistor)
            pinMode(SCL, INPUT_PULLUP);
            pinMode(SDA, INPUT_PULLUP);
            delay(10);

            Wire.begin(iic_addrs::SlaveMCU);
            delay(300);

            LOG_DONE();

            { // MasterMCU
                LOG_INFO_START("Initializing IIC Commu");
                if constexpr (initializing_list.MasterBoard) {
                    iic_commu::begin();
                    LOG_DONE();
                } else
                    LOG_SKIP();
            }

        } else
            LOG_SKIP();
    }

    { // oled1362
        LOG_INFO_START("Initializing OLED1362");
        if (initializing_list.OLED1362) {
            if (!oled1362.begin())
                LOG_FAIL();
            else {
                oled1362.enable();
                LOG_DONE();
            }
        } else
            LOG_SKIP();
    }

    { // oled1306
        LOG_INFO_START("Initializing OLED1306");
        if (initializing_list.OLED1306) {
            if (!oled1306.begin())
                LOG_FAIL();
            else {
                oled1306.enable();
                LOG_DONE();
            }
        } else
            LOG_SKIP();
    }

    { // Wifi
        if (initializing_list.WiFi) {
            LOG_INFO("Initializing WiFi");
            if (udp_comm.begin() != 0) {
            } else {
            }
        }
    }

    { // IR
        LOG_INFO_START("Initializing IR");
        if (initializing_list.IR) {

            qtr.setTypeRC();
            qtr.setSensorPins((const uint8_t[]){
                              IR_READ_1, IR_READ_2, IR_READ_3,
                              IR_READ_4, IR_READ_5, IR_READ_6,
                              IR_READ_7, IR_READ_8, IR_READ_9 },
            IR_CONUT);

            qtr.setEmitterPins(IR_CTRL, IR_CTRL);

            delay(400);

            for (int i = 0; i < 400; i++) {
                qtr.calibrate();
            }

            LOG_DONE();

            for (uint8_t i = 0; i < IR_CONUT; i++)
                LOG_DEBUG("     qtr.calibrationOn.minimum[{}]: ", i, qtr.calibrationOn.minimum[i]);
            for (uint8_t i = 0; i < IR_CONUT; i++)
                LOG_DEBUG("     qtr.calibrationOn.maximum[{}]: ", i, qtr.calibrationOn.maximum[i]);

        } else
            LOG_SKIP();
    }

    // finished
    LOG_INFO("Peripherals initialization finished.");
}

} // namespace peripherals