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
                LOG_INFO_START("Initializing MasterBoard IIC Commu");
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

    // finished
    LOG_INFO("Peripherals initialization finished.");
}

} // namespace peripherals