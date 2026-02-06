// peripherals.cpp
#include "dev/peripherals.hpp"

namespace peripherals {

// dev_oled1362 oled1362{ OLED_CS, OLED_DC, OLED_RESET }; // SPI
// dev_oled1306 oled1306{};                               // IIC

ModulinoButtons buttons{}; // IIC
ModulinoBuzzer buzzer{};   // IIC
ModulinoMovement imu{};    // IIC
ModulinoKnob knob{};       // IIC
ModulinoPixels pixels{};   // IIC

Motor motor_l{ ENCODERL_A, ENCODERL_B, MOTOR_L_DIR, MOTOR_L_EN }; // interrupt
Motor motor_r{ ENCODERR_A, ENCODERR_B, MOTOR_R_DIR, MOTOR_R_EN }; // interrupt

auto begin() -> void {

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
                    LOG_INFO("Found I2C device at 0x{h}", addr);
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

    // { // oled1362
    //     LOG_INFO_START("Initializing OLED1362");
    //     if (enable_list.OLED1362) {
    //         if (!oled1362.begin())
    //             LOG_FAIL();
    //         else {
    //             oled1362.enable();
    //             LOG_DONE();
    //         }
    //     } else
    //         LOG_SKIP();
    // }

    // { // oled1306
    //     LOG_INFO_START("Initializing OLED1306");
    //     if (enable_list.OLED1306) {
    //         if (!oled1306.begin())
    //             LOG_FAIL();
    //         else {
    //             oled1306.enable();
    //             LOG_DONE();
    //         }
    //     } else
    //         LOG_SKIP();
    // }

    { // buttons
        LOG_INFO_START("Initializing ModulinoButtons");
        if (enable_list.Buttons) {
            if (!buttons.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // imu
        LOG_INFO_START("Initializing ModulinoIMU");
        if (enable_list.IMU) {
            if (!imu.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // knob
        LOG_INFO_START("Initializing ModulinoKnob");
        if (enable_list.Knob) {
            if (!knob.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }


    { // pixels
        LOG_INFO_START("Initializing ModulinoPixels");
        if (enable_list.Pixels) {
            if (!pixels.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // buzzer
        LOG_INFO_START("Initializing ModulinoBuzzer");
        if (enable_list.Buzzer) {
            if (!buzzer.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // motor
        LOG_INFO_START("Initializing Motor (L & R) and Encoder");
        if (enable_list.Motor) {
            if (!motor_l.begin() && motor_r.begin())
                LOG_FAIL();
            else
                LOG_DONE();
        } else
            LOG_SKIP();
    }

    // finished
    LOG_INFO("Peripherals initialization finished.");
}

} // namespace peripherals