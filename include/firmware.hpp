#pragma once

#include <Arduino.h>
#include <Modulino.h>

#include "dev/oled1306.hpp"
#include "dev/oled1362.hpp"
#include "logger.hpp"


constexpr uint8_t ENCODERL_A = D2;
constexpr uint8_t ENCODERL_B = D3;
constexpr uint8_t ENCODERR_A = D7;
constexpr uint8_t ENCODERR_B = D8;

constexpr uint8_t OLED_CS  = D10;
constexpr uint8_t OLED_RST = D9;
constexpr uint8_t OLED_DC  = D4;

dev_oled1362 oled1362{ OLED_CS, OLED_DC, OLED_RST }; // SPI
dev_oled1306 oled1306{};                             // IIC

ModulinoButtons buttons{}; // IIC
ModulinoBuzzer buzzer{};   // IIC
ModulinoMovement imu{};    // IIC
ModulinoKnob knob{};       // IIC
ModulinoPixels pixels{};   // IIC

struct {

    bool IIC      = false;
    bool Modulino = false;
    bool OLED1362 = false;
    bool OLED1306 = false;
    bool buttons  = false;
    bool pixels   = false;
    bool knob     = false;
    bool imu      = false;
    bool buzzer   = false;

} enable_list __packed;

auto firmware_begin() -> void {


    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
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
                    LOG_INFO("Found I2C device at 0x{}", addr);
                }
            }
        } else
            LOG_SKIP();
    }

    { // Modulino
        LOG_INFO_START("Initializing Modulino");
        if (enable_list.Modulino) {
            pinMode(SCL, INPUT_PULLUP);
            pinMode(SDA, INPUT_PULLUP);
            delay(10);
            Modulino.begin();
            delay(100); // essential
            LOG_DONE();
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

    { // buttons
        LOG_INFO_START("Initializing ModulinoButtons");
        if (enable_list.buttons) {
            if (!buttons.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // imu
        LOG_INFO_START("Initializing ModulinoIMU");
        if (enable_list.imu) {
            if (!imu.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // knob
        LOG_INFO_START("Initializing ModulinoKnob");
        if (enable_list.knob) {
            if (!knob.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }


    { // pixels
        LOG_INFO_START("Initializing ModulinoPixels");
        if (enable_list.pixels) {
            if (!pixels.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // buzzer
        LOG_INFO_START("Initializing ModulinoBuzzer");
        if (enable_list.buzzer) {
            if (!buzzer.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }
}
