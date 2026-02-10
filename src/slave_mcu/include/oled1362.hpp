// oled1362.hpp
#pragma once

#include <Arduino.h>
#include <SPI.h>

#include <U8g2lib.h>

class dev_oled1362 {
    public:
    dev_oled1362(uint8_t cs = 10, uint8_t dc = 8, uint8_t rst = 9);

    bool begin();
    bool is_enable();
    void enable();
    void disable();
    inline const char* get_name() { return "OLED_SSD1362"; }

    U8G2& get_disp() { return m_u8g2; }

    void clear();
    void disp_text(const char* text, uint8_t x = 0, uint8_t y = 20);
    void disp_status(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr);
    void set_contrast(uint8_t value);
    void set_brightness(uint8_t value) { set_contrast(value); }

    private:
    U8G2_SSD1362_256X64_F_4W_HW_SPI m_u8g2;
    bool m_initialized;
    bool m_is_enable;
    uint8_t m_cs_pin;
    uint8_t m_dc_pin;
    uint8_t m_rst_pin;
};