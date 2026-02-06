#pragma once

#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>

#include "dev/oled1306.hpp"
#include "dev/oled1362.hpp"

#include "serial_logger.hpp"

namespace peripherals {

// constexpr uint8_t ENCODERL_A = A0;
// constexpr uint8_t ENCODERL_B = A1;
// constexpr uint8_t ENCODERR_A = A2;
// constexpr uint8_t ENCODERR_B = A3;
constexpr uint8_t IIC_DTA = A4;
constexpr uint8_t IIC_SCL = A5;

constexpr uint8_t SERIAL_Rx = D0;
constexpr uint8_t SERIAL_Tx = D1;
// constexpr uint8_t MOTOR_L_DIR = D2;
// constexpr uint8_t MOTOR_L_EN  = D3;
// constexpr uint8_t MOTOR_R_DIR = D4;
// constexpr uint8_t MOTOR_R_EN  = D5;
// constexpr uint8_t ENCODERL_A  = D6;
// constexpr uint8_t ENCODERL_B  = D7;
constexpr uint8_t OLED_DC  = D8;
constexpr uint8_t OLED_RST = D9;
constexpr uint8_t OLED_CS  = D10;
constexpr uint8_t SPI_MOSI = D11;
constexpr uint8_t SPI_MISO = D12;
constexpr uint8_t SPI_SCK  = D13;

extern dev_oled1362 oled1362; // SPI
extern dev_oled1306 oled1306; // IIC

struct {

    bool IIC      = false;
    bool SPI      = true;
    bool OLED1362 = false;
    bool OLED1306 = false;

} enable_list __packed;

auto begin() -> void;

} // namespace peripherals