#include <Arduino.h>

#include "dev/manager.hpp"
#include "dev/oled1362.hpp"

#include "logger.hpp"

dev_oled1362 oled(10, 8, 9); // CS=10, DC=8, RST=9, SPI

void setup() {

    LOG_BEGIN(115200);
    LOG_SETSHOWLEVEL(true);
    LOG_SETSHOWLOCATION(true);

    device_manager& dm = device_manager::get_instance();

    dm.reg_device(&oled, "display");

    dm.initialize_all();
    dm.list_devices();
    dev_base* device = dm.get_device("display");
    if (device) {
        dev_oled1362* oledPtr = static_cast<dev_oled1362*>(device);
        oledPtr->disp_status("Device Manager", "OLED Active", "Ready!");
    }

    delay(2000);
}

void loop() {
    static uint32_t counter    = 0;
    static uint32_t lastUpdate = 0;

    // 每秒更新一次显示
    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();

        device_manager& dm = device_manager::get_instance();

        // 获取OLED并显示计数器
        dev_base* device = dm.get_device("display");
        if (device && device->is_enable()) {
            dev_oled1362* oledPtr = static_cast<dev_oled1362*>(device);

            char line1[32];
            char line2[32];
            sprintf(line1, "Count: %lu", counter++);
            sprintf(line2, "Time: %lu s", millis() / 1000);

            oledPtr->disp_status("Device Manager", line1, line2);
        }
    }
}