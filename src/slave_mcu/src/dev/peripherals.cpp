// peripherals.cpp
#include "dev/peripherals.hpp"

namespace peripherals {

dev_oled1362 oled1362{ OLED_CS, OLED_DC, OLED_RST }; // SPI
dev_oled1306 oled1306{};                             // IIC

auto begin() -> void {

    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
    }

    { // SPI
        LOG_INFO_START("Initializing SPI");
        if (enable_list.SPI) {
            delay(10);
            SPI.begin();
            delay(300);
            LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // iic
        LOG_INFO_START("Initializing IIC");
        if (enable_list.IIC) {
            // iic pinmode (pullup resistor)
            pinMode(SCL, INPUT_PULLUP);
            pinMode(SDA, INPUT_PULLUP);
            delay(10);
            Wire.begin();
            delay(300);
            LOG_DONE();

            for (byte addr = 0x01; addr < 0x7F; addr++) {
                Wire.beginTransmission(addr);
                byte error = Wire.endTransmission();
                if (error == 0) {
                    LOG_INFO("Found I2C device at 0x{h}", addr);
                }
            }
        } else
            LOG_SKIP();
    }

    { // oled1362
        LOG_INFO_START("Initializing OLED1362");
        if (enable_list.OLED1362) {
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
        if (enable_list.OLED1306) {
            if (!oled1306.begin())
                LOG_FAIL();
            else {
                oled1306.enable();
                LOG_DONE();
            }
        } else
            LOG_SKIP();
    }

    // finished
    LOG_INFO("Peripherals initialization finished.");
}

} // namespace peripherals