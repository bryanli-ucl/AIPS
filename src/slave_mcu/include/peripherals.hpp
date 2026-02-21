#pragma once

#include <Arduino.h>

#include <SPI.h>
#include <WiFiUdp.h>
#include <Wire.h>

#include "common.hpp"
#include "iic_commu.hpp"
#include "oled1306.hpp"
#include "oled1362.hpp"

#include "serial_logger.hpp"

namespace peripherals {

constexpr uint8_t IR_READ_6 = A0;
constexpr uint8_t IR_READ_7 = A1;
constexpr uint8_t IR_READ_8 = A2;
constexpr uint8_t IR_READ_9 = A3;
constexpr uint8_t IIC_DTA   = A4;
constexpr uint8_t IIC_SCL   = A5;

constexpr uint8_t SERIAL_Rx = D0;
constexpr uint8_t SERIAL_Tx = D1;
constexpr uint8_t IR_READ_1 = D2;
constexpr uint8_t IR_READ_2 = D3;
constexpr uint8_t IR_READ_3 = D4;
constexpr uint8_t IR_READ_4 = D5;
constexpr uint8_t SERVO_PIN = D6;
constexpr uint8_t IR_READ_5 = D7;
constexpr uint8_t OLED_RST  = D8;
constexpr uint8_t OLED_DC   = D9;
constexpr uint8_t OLED_CS   = D10;
constexpr uint8_t SPI_MOSI  = D11;
constexpr uint8_t SPI_MISO  = D12;
constexpr uint8_t SPI_SCK   = D13;

constexpr char* NETWORK_SSID        = "bryan";
constexpr char* NETWORK_PASSWORD    = "12345678";
constexpr uint16_t NETWORK_UDP_PORT = 1145;

extern dev_oled1362 oled1362; // SPI
extern dev_oled1306 oled1306; // IIC
extern WiFiUDP udp;           // WiFi

struct {

    bool SPI      = false;
    bool OLED1362 = false;

    bool IIC         = true;
    bool OLED1306    = false;
    bool MasterBoard = true;

    bool LiDAR = false;
    bool IR    = false;
    bool Servo = false;

    bool WiFi = true;

} constexpr initializing_list;

auto begin() -> void;

} // namespace peripherals