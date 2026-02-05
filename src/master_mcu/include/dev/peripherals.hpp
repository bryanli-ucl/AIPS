#pragma once

#include <Arduino.h>
#include <Modulino.h>

#include "dev/motor.hpp"
#include "dev/oled1306.hpp"
#include "dev/oled1362.hpp"

#include "serial_logger.hpp"

namespace peripherals {

constexpr uint8_t ENCODERL_A = D2;
constexpr uint8_t ENCODERL_B = D4;
constexpr uint8_t ENCODERR_A = D7;
constexpr uint8_t ENCODERR_B = D8;

constexpr uint8_t OLED_CS  = D10;
constexpr uint8_t OLED_RST = D9;
constexpr uint8_t OLED_DC  = D12;

extern dev_oled1362 oled1362; // SPI
extern dev_oled1306 oled1306; // IIC

extern ModulinoButtons buttons; // IIC
extern ModulinoBuzzer buzzer;   // IIC
extern ModulinoMovement imu;    // IIC
extern ModulinoKnob knob;       // IIC
extern ModulinoPixels pixels;   // IIC

extern Motor motor_l; // interrupt
extern Motor motor_r; // interrupt

struct {

    bool IIC      = false;
    bool Modulino = false;
    bool OLED1362 = false;
    bool OLED1306 = false;
    bool Buttons  = false;
    bool Pixels   = false;
    bool Knob     = false;
    bool IMU      = false;
    bool Buzzer   = false;
    bool Motor    = true;

} enable_list __packed;

auto begin() -> void;

} // namespace peripherals