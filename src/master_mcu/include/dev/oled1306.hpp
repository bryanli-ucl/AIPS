// oled1306.hpp
#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class dev_oled1306 {
    public:
    dev_oled1306(uint8_t width = 128, uint8_t height = 64, uint8_t iic_addr = 0x3C);

    bool begin();
    bool is_enable();
    void enable();
    void disable();
    inline const char* get_name() { return "OLED_SSD1306"; }

    Adafruit_SSD1306& get_disp() { return m_display; }

    void clear();
    void disp_text(const char* text, uint8_t x = 0, uint8_t y = 20);
    void disp_status(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr);
    void set_contrast(uint8_t value);
    void set_brightness(uint8_t value) { set_contrast(value); }

    private:
    Adafruit_SSD1306 m_display;
    bool m_initialized;
    bool m_is_enable;

    uint8_t m_i2c_addr;
    int8_t m_rst_pin;

    uint8_t m_width;
    uint8_t m_height;
};
