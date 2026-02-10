// oled1306.cpp
#include "oled1306.hpp"


dev_oled1306::dev_oled1306(uint8_t width, uint8_t height, uint8_t i2c_addr)
: m_display(width, height, &Wire, -1),
  m_initialized(false),
  m_is_enable(false),
  m_i2c_addr(i2c_addr),
  m_width(width),
  m_height(height) {}


bool dev_oled1306::begin() {
    if (m_initialized) {
        return true;
    }

    m_initialized = m_display.begin(SSD1306_SWITCHCAPVCC, m_i2c_addr);

    if (m_initialized) {
        m_is_enable = true;
        clear();
        m_display.display();
    }

    return m_initialized;
}


bool dev_oled1306::is_enable() {
    return m_is_enable;
}

void dev_oled1306::enable() {
    if (m_initialized) {
        m_display.ssd1306_command(SSD1306_DISPLAYON);
        m_is_enable = true;
    }
}

void dev_oled1306::disable() {
    if (m_initialized) {
        m_display.ssd1306_command(SSD1306_DISPLAYOFF);
        m_is_enable = false;
    }
}

void dev_oled1306::clear() {
    m_display.clearDisplay();
    m_display.display();
}

void dev_oled1306::disp_text(const char* text, uint8_t x, uint8_t y) {
    if (!m_initialized || !m_is_enable || !text) {
        return;
    }

    m_display.clearDisplay();
    m_display.setTextSize(1);
    m_display.setTextColor(SSD1306_WHITE);
    m_display.setCursor(x, y);
    m_display.print(text);
    m_display.display();
}

void dev_oled1306::disp_status(const char* line1, const char* line2, const char* line3) {
    if (!m_initialized || !m_is_enable) {
        return;
    }

    m_display.clearDisplay();
    m_display.setTextSize(1);
    m_display.setTextColor(SSD1306_WHITE);

    if (line1) {
        m_display.setCursor(0, 0);
        m_display.print(line1);
    }

    if (line2) {
        m_display.setCursor(0, 20);
        m_display.print(line2);
    }

    if (line3) {
        m_display.setCursor(0, 40);
        m_display.print(line3);
    }

    m_display.display();
}

void dev_oled1306::set_contrast(uint8_t value) {
    if (m_initialized) {
        m_display.ssd1306_command(SSD1306_SETCONTRAST);
        m_display.ssd1306_command(value);
    }
}