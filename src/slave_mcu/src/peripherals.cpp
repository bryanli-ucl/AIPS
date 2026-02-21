// peripherals.cpp
#include "peripherals.hpp"

#include <WiFiS3.h>

namespace peripherals {

dev_oled1362 oled1362{ OLED_CS, OLED_DC, OLED_RST }; // SPI
dev_oled1306 oled1306{};                             // IIC

WiFiUDP udp{}; // Serial WiFi

auto begin() -> void {

    { // SPI
        LOG_INFO_START("Initializing SPI");
        if (initializing_list.SPI) {
            delay(10);
            SPI.begin();
            delay(300);
            LOG_DONE();
        } else
            LOG_SKIP();
    }

    { // iic
        LOG_INFO_START("Initializing IIC");
        if (initializing_list.IIC) {
            // iic pinmode (pullup resistor)
            pinMode(SCL, INPUT_PULLUP);
            pinMode(SDA, INPUT_PULLUP);
            delay(10);

            Wire.begin(iic_addrs::SlaveMCU);
            delay(300);

            LOG_DONE();

            { // MasterMCU
                LOG_INFO_START("Initializing IIC Commu");
                if constexpr (initializing_list.MasterBoard) {
                    iic_commu::begin();
                    LOG_DONE();
                } else
                    LOG_SKIP();
            }

        } else
            LOG_SKIP();
    }

    { // oled1362
        LOG_INFO_START("Initializing OLED1362");
        if (initializing_list.OLED1362) {
            if (!oled1362.begin())
                LOG_FAIL();
            else {
                oled1362.enable();
                LOG_DONE();
            }
        } else
            LOG_SKIP();
    }

    { // oled1306
        LOG_INFO_START("Initializing OLED1306");
        if (initializing_list.OLED1306) {
            if (!oled1306.begin())
                LOG_FAIL();
            else {
                oled1306.enable();
                LOG_DONE();
            }
        } else
            LOG_SKIP();
    }

    { // Wifi
        LOG_INFO_START("Initializing WiFi");
        if (initializing_list.WiFi) {

            // ====== 第一步：检查模块 ======
            Serial.print("检查 WiFi 模块... ");
            if (WiFi.status() == WL_NO_MODULE) {
                Serial.println("失败！模块未找到");
                LOG_FAIL();
                return;
            }
            Serial.println("OK");

            // ====== 第二步：打印固件版本 ======
            String fv = WiFi.firmwareVersion();
            Serial.print("固件版本: ");
            Serial.println(fv);

            // ====== 第三步：扫描网络（确认能找到你的 WiFi）======
            Serial.println("扫描网络...");
            int n = WiFi.scanNetworks();
            Serial.print("找到 ");
            Serial.print(n);
            Serial.println(" 个网络");

            bool found = false;
            for (int i = 0; i < n; i++) {
                String ssid = WiFi.SSID(i);
                Serial.print("  ");
                Serial.print(i);
                Serial.print(": ");
                Serial.print(ssid);
                Serial.print(" (");
                Serial.print(WiFi.RSSI(i));
                Serial.println(" dBm)");

                if (ssid == String(NETWORK_SSID)) {
                    found = true;
                    Serial.println("    ★ 这是目标网络");
                }
            }

            if (!found) {
                Serial.println("⚠️ 警告：未找到目标 SSID！");
                Serial.print("目标 SSID: '");
                Serial.print(NETWORK_SSID);
                Serial.println("'");
            }

            // ====== 第四步：尝试连接 ======
            Serial.println("\n开始连接...");
            Serial.print("SSID: ");
            Serial.println(NETWORK_SSID);

            int status = WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);

            Serial.print("WiFi.begin() 返回值: ");
            Serial.println(status);

            // 不要立即判断失败，等待一下
            if (status == WL_CONNECT_FAILED) {
                Serial.println("WiFi.begin() 返回失败");
                // 但还是尝试等待
            }

            // ====== 第五步：等待连接 ======
            LOG_DONE();
            Serial.print("等待连接");

            int attempts           = 0;
            const int MAX_ATTEMPTS = 40; // 20 秒

            while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS) {
                delay(500);
                Serial.print(".");

                if (attempts % 10 == 0) {
                    Serial.print(" [状态:");
                    Serial.print(WiFi.status());
                    Serial.print("] ");
                }

                attempts++;
            }
            Serial.println();

            // ====== 第六步：检查结果 ======
            int finalStatus = WiFi.status();
            Serial.print("最终状态: ");
            Serial.print(finalStatus);
            Serial.print(" - ");

            switch (finalStatus) {
            case WL_CONNECTED:
                while (WiFi.localIP()[0] == 0)
                    delay(100);
                Serial.println("已连接");
                Serial.print("IP: ");
                Serial.println(WiFi.localIP());
                udp.begin(NETWORK_UDP_PORT);
                Serial.print("UDP 端口: ");
                Serial.println(NETWORK_UDP_PORT);
                break;

            case WL_NO_SSID_AVAIL:
                Serial.println("找不到 SSID");
                LOG_FAIL();
                break;

            case WL_CONNECT_FAILED:
                Serial.println("连接失败（密码错误？）");
                LOG_FAIL();
                break;

            case WL_IDLE_STATUS:
                Serial.println("空闲");
                LOG_FAIL();
                break;

            case WL_DISCONNECTED:
                Serial.println("已断开");
                LOG_FAIL();
                break;

            default:
                Serial.println("未知状态");
                LOG_FAIL();
                break;
            }

        } else
            LOG_SKIP();
    }

    // finished
    LOG_INFO("Peripherals initialization finished.");
}

} // namespace peripherals