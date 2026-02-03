#include <Arduino.h>
#include <Modulino.h>

#include <exception>

#include "dev/oled1306.hpp"
#include "dev/oled1362.hpp"

#include "logger.hpp"

dev_oled1362 oled1362{ 10, 8, 9 }; // CS=10, DC=8, RST=9, SPI
dev_oled1306 oled1306{};           // IIC
ModulinoButtons buttons{};         // IIC
ModulinoBuzzer buzzer{};           // IIC
ModulinoMovement imu{};            // IIC
ModulinoKnob knob{};               // IIC
ModulinoPixels pixels{};           // IIC


auto scan_iic() -> void {
    LOG_INFO("Scanning IIC bus...");
    Wire.begin();

    byte error, address;
    int nDevices = 0;
    for (address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        LOG_TRACE("IIC Scan addr: {}, error: {}", address, error);

        if (error == 0) {
            LOG_INFO("IIC device found at address 0x{}", address);
            nDevices++;
        }
    }
    LOG_INFO("Found {} I2C devices", nDevices);
}

auto setup() -> void {

    LOG_BEGIN();

    try {

        scan_iic();

        // oled1362
        LOG_INFO_START("Initializing OLED1362");
        if (!oled1362.begin()) throw std::runtime_error("cannot init OLED1362");
        oled1362.enable();
        LOG_DONE();

        // Modulino
        LOG_INFO_START("Initializing Modulino");
        Modulino.begin();
        LOG_DONE();

        // oled1306
        LOG_INFO_START("Initializing OLED1306");
        if (!oled1306.begin()) throw std::runtime_error("cannot init OLED1306");
        oled1306.enable();
        LOG_DONE();

        // buttons
        LOG_INFO_START("Initializing ModulinoButtons");
        if (!buttons.begin()) throw std::runtime_error("cannot init buttons");
        LOG_DONE();

        // imu
        LOG_INFO_START("Initializing ModulinoIMU");
        if (!imu.begin()) throw std::runtime_error("cannot init IMU");
        LOG_DONE();

        // knob
        LOG_INFO_START("Initializing ModulinoKnob");
        if (!knob.begin()) throw std::runtime_error("cannot init Knob");
        LOG_DONE();

        // pixels
        LOG_INFO_START("Initializing ModulinoPixels");
        if (!pixels.begin()) throw std::runtime_error("cannot init Pixels");
        LOG_DONE();

        // buzzers
        // LOG_INFO_START("Initializing ModulinoBuzzer");
        // if (!buzzer.begin()) throw std::runtime_error("cannot init buzzer");
        // LOG_DONE();
        // LOG_INFO("buzzer addr: {}", reinterpret_cast<unsigned long long>(buzzer.getWire()));


    } catch (std::exception& e) {
        LOG_FATAL("{}", e.what());

        pinMode(LED_BUILTIN, OUTPUT);
        while (true) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(400);
            digitalWrite(LED_BUILTIN, LOW);
            delay(400);
        }
    }
}

enum class working_status_t : uint8_t {
    IDLE
};

auto loop() -> void {

    static uint32_t counter    = 0;
    static uint32_t lastUpdate = 0;

    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();

        if (oled1362.is_enable()) {
            LOG_INFO("change disp message 1362");

            char line1[32];
            char line2[32];
            char line3[32];
            sprintf(line1, "Count: %lu", counter++);
            sprintf(line2, "Time: %lu s", millis() / 1000);
            sprintf(line3, "Menu: %s", "Device DISP");

            oled1362.disp_status(line3, line1, line2);
        }

        if (oled1306.is_enable()) {
            LOG_INFO("change disp message 1306");
            oled1306.disp_status("Bryan", "hello", "world");
        }
    }
}