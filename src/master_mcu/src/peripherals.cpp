// peripherals.cpp
#include "peripherals.hpp"

namespace peripherals {

ModulinoButtons buttons{}; // IIC0
ModulinoBuzzer buzzer{};   // IIC0
ModulinoMovement imu{};    // IIC0
ModulinoKnob knob{};       // IIC0
ModulinoPixels pixels{};   // IIC0

MotoronI2C motoron{}; // IIC1

Motor motor_l{ ENCODERL_A, ENCODERL_B, MOTOR_L_NUM, motoron }; // interrupt
Motor motor_r{ ENCODERR_A, ENCODERR_B, MOTOR_R_NUM, motoron }; // interrupt

master_iic_data_t master_data;
slave_iic_data_t slave_data;

auto begin() -> void {

    { // Modulino
        LOG_INFO_START("Initializing Modulino");
        if constexpr (initializing_list.Modulino) {
            Modulino.begin();
            delay(100); // essential
            LOG_DONE();

            { // buttons
                LOG_INFO_START("Initializing ModulinoButtons");
                if constexpr (initializing_list.Buttons) {
                    if (!buttons.begin())
                        LOG_FAIL();
                    else
                        LOG_DONE();
                } else
                    LOG_SKIP();
            }

            { // imu
                LOG_INFO_START("Initializing ModulinoIMU");
                if constexpr (initializing_list.IMU) {
                    if (!imu.begin())
                        LOG_FAIL();
                    else
                        LOG_DONE();
                } else
                    LOG_SKIP();
            }

            { // knob
                LOG_INFO_START("Initializing ModulinoKnob");
                if constexpr (initializing_list.Knob) {
                    if (!knob.begin())
                        LOG_FAIL();
                    else
                        LOG_DONE();
                } else
                    LOG_SKIP();
            }


            { // pixels
                LOG_INFO_START("Initializing ModulinoPixels");
                if constexpr (initializing_list.Pixels) {
                    if (!pixels.begin())
                        LOG_FAIL();
                    else
                        LOG_DONE();
                } else
                    LOG_SKIP();
            }

            { // buzzer
                LOG_INFO_START("Initializing ModulinoBuzzer");
                if constexpr (initializing_list.Buzzer) {
                    if (!buzzer.begin())
                        LOG_FAIL();
                    else
                        LOG_DONE();
                } else
                    LOG_SKIP();
            }

        } else
            LOG_SKIP();
    }

    { // IIC1
        LOG_INFO_START("Initializing IIC1");
        if constexpr (initializing_list.IIC) {
            // iic pinmode (pullup resistor)
            pinMode(SCL, INPUT_PULLUP);
            pinMode(SDA, INPUT_PULLUP);
            delay(10);
            Wire.begin();
            delay(300);
            LOG_DONE();

            // scanning iic address
            if constexpr (true) {
                for (byte addr = 0x01; addr < 0x20; addr++) {
                    Wire.beginTransmission(addr);
                    byte error = Wire.endTransmission();
                    if (error == 0) {
                        LOG_INFO("Found I2C device at 0x{h}", addr);
                    } else {
                        LOG_INFO("No I2C device at 0x{h}", addr);
                    }
                }
            }

            // Motoron
            if (Wire.beginTransmission(static_cast<uint8_t>(iic_addrs::Motoron)), Wire.endTransmission() == 0)
                LOG_INFO("Motoron Connnected");
            else
                LOG_WARN("Motoron Unconnnected");

            // SlaveMCU
            if (Wire.beginTransmission(static_cast<uint8_t>(iic_addrs::SlaveMCU)), Wire.endTransmission() == 0)
                LOG_INFO("SlaveMCU Connnected");
            else
                LOG_WARN("SlaveMCU Unconnnected");

        } else
            LOG_SKIP();
    }


    { // motoron
        LOG_INFO_START("Initializing Motor Controller");
        if constexpr (initializing_list.Motoron) {

            motoron.reinitialize(); // Bytes: 0x96 0x74
            motoron.disableCrc();   // Bytes: 0x8B 0x04 0x7B 0x43

            // Clear the reset flag, which is set after the controller
            // reinitializes and counts as an error.
            motoron.clearResetFlag(); // Bytes: 0xA9 0x00 0x04

            motoron.setMaxAcceleration(1, 140);
            motoron.setMaxDeceleration(1, 300);
            motoron.setMaxAcceleration(2, 140);
            motoron.setMaxDeceleration(2, 300);

            LOG_DONE();

            { // motor
                LOG_INFO_START("Initializing Motor L & R");
                if constexpr (initializing_list.Motor) {
                    if (!motor_l.begin() && motor_r.begin())
                        LOG_FAIL();
                    else
                        LOG_DONE();
                } else
                    LOG_SKIP();
            }

        } else
            LOG_SKIP();
    }

    // finished
    LOG_INFO("Peripherals initialization finished.");
}

} // namespace peripherals