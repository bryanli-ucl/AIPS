#pragma once

#include <Arduino.h>

#include <Modulino.h>
#include <Motoron.h>

#include <SPI.h>
#include <Wire.h>

#include "common.hpp"
#include "motor.hpp"
#include "serial_logger.hpp"

namespace peripherals {

constexpr uint8_t _P_A0   = A0;
constexpr uint8_t _P_A1   = A1;
constexpr uint8_t _P_A2   = A2;
constexpr uint8_t _P_A3   = A3;
constexpr uint8_t IIC_DTA = A4;
constexpr uint8_t IIC_SCL = A5;

constexpr uint8_t SERIAL_Rx  = D0;
constexpr uint8_t SERIAL_Tx  = D1;
constexpr uint8_t ENCODERL_A = D2;
constexpr uint8_t ENCODERR_A = D3;
constexpr uint8_t ENCODERL_B = D4;
constexpr uint8_t ENCODERR_B = D5;
constexpr uint8_t _P_D6      = D6;
constexpr uint8_t _P_D7      = D7;
constexpr uint8_t _P_D8      = D8;
constexpr uint8_t _P_D9      = D9;
constexpr uint8_t _P_D10     = D10;
constexpr uint8_t _P_D11     = D11;
constexpr uint8_t _P_D12     = D12;
constexpr uint8_t _P_D13     = D13;

constexpr uint8_t MOTOR_L_NUM = 1;
constexpr uint8_t MOTOR_R_NUM = 2;

extern ModulinoButtons buttons; // IIC0
extern ModulinoBuzzer buzzer;   // IIC0
extern ModulinoMovement imu;    // IIC0
extern ModulinoKnob knob;       // IIC0
extern ModulinoPixels pixels;   // IIC0

extern MotoronI2C motoron; // IIC1

extern Motor motor_l; // interrupt
extern Motor motor_r; // interrupt


extern master_iic_data_t master_data;
extern slave_iic_data_t slave_data;

struct {

    bool Modulino = false;
    bool Buttons  = true;
    bool Pixels   = true;
    bool Knob     = true;
    bool IMU      = true;
    bool Buzzer   = false;
    bool IIC      = true;
    bool Motor    = true;
    bool Motoron  = true;

} constexpr initializing_list;

auto begin() -> void;

} // namespace peripherals