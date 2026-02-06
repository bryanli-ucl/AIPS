#pragma once

#include <Arduino.h>
#include <Modulino.h>

#include "dev/motor.hpp"

#include "serial_logger.hpp"

namespace peripherals {

// constexpr uint8_t ENCODERL_A = A0;
// constexpr uint8_t ENCODERL_B = A1;
// constexpr uint8_t ENCODERR_A = A2;
// constexpr uint8_t ENCODERR_B = A3;
constexpr uint8_t IIC_DTA = A4;
constexpr uint8_t IIC_SCL = A5;

constexpr uint8_t SERIAL_Rx   = D0;
constexpr uint8_t SERIAL_Tx   = D1;
constexpr uint8_t MOTOR_L_DIR = D2;
constexpr uint8_t MOTOR_L_EN  = D3;
constexpr uint8_t MOTOR_R_DIR = D4;
constexpr uint8_t MOTOR_R_EN  = D5;
constexpr uint8_t ENCODERL_A  = D6;
constexpr uint8_t ENCODERL_B  = D7;
constexpr uint8_t ENCODERR_A  = D8;
constexpr uint8_t ENCODERR_B  = D9;
constexpr uint8_t SPI_SS      = D10;
constexpr uint8_t SPI_MOSI    = D11;
constexpr uint8_t SPI_MISO    = D12;
constexpr uint8_t SPI_SCK     = D13;

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