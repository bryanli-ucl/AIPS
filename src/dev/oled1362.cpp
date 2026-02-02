#include "dev/oled1362.hpp"

#include "logger.hpp"

dev_oled1362::dev_oled1362(uint8_t cs, uint8_t dc, uint8_t rst)
: m_u8g2(U8G2_R0, cs, dc, rst), m_is_enable(false), m_cs_pin(cs), m_dc_pin(dc), m_rst_pin(rst), m_initialized(false) {
}

bool dev_oled1362::begin() {
    LOG_INFO("initializing {} ...", get_name());

    m_u8g2.begin();
    m_u8g2.setBusClock(8000000); // 8MHz
    m_u8g2.setContrast(128);

    m_u8g2.clearBuffer();
    m_u8g2.sendBuffer();

    m_initialized = true;
    m_is_enable   = true;

    LOG_INFO("DONE");

    disp_status("OLED Ready", "SSD1362", "256x64");
    delay(1000);

    return true;
}

void dev_oled1362::update() {
    if (!m_is_enable || !m_initialized) {
        return;
    }

    // ...
}

void dev_oled1362::enable(bool active) {
    m_is_enable = active;

    if (!active) {
        m_u8g2.setPowerSave(1);
    } else if (m_initialized) {
        m_u8g2.setPowerSave(0);
    }
}

void dev_oled1362::clear() {
    if (!m_initialized) return;

    m_u8g2.clearBuffer();
    m_u8g2.sendBuffer();
}

void dev_oled1362::disp_text(const char* text, uint8_t x, uint8_t y) {
    if (!m_initialized || !m_is_enable) return;

    m_u8g2.clearBuffer();
    m_u8g2.setFont(u8g2_font_ncenB10_tr);
    m_u8g2.drawStr(x, y, text);
    m_u8g2.sendBuffer();
}

void dev_oled1362::disp_status(const char* line1, const char* line2, const char* line3) {
    if (!m_initialized || !m_is_enable) return;

    m_u8g2.clearBuffer();
    m_u8g2.setFont(u8g2_font_ncenB10_tr);

    if (line1) {
        m_u8g2.drawStr(10, 15, line1);
    }

    if (line2) {
        m_u8g2.drawStr(10, 35, line2);
    }

    if (line3) {
        m_u8g2.drawStr(10, 55, line3);
    }

    m_u8g2.sendBuffer();
}

void dev_oled1362::set_contrast(uint8_t value) {
    if (!m_initialized) return;
    m_u8g2.setContrast(value);
}
