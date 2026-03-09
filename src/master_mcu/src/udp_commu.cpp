#include "udp_commu.hpp"

#include <Arduino.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>

#include "serial_logger.hpp"

WiFiUDP udp{}; // Serial WiFi

WifiCommu::WifiCommu(char* net_ssid, char* net_pwd, uint16_t udp_port)
: m_net_ssid(net_ssid), m_net_pwd(net_pwd), m_udp_port(udp_port) {
}

int8_t WifiCommu::begin() {

    LOG_INFO_START("CHECK WIFI MODULE");
    if (WiFi.status() == WL_NO_MODULE) {
        LOG_FAIL();
        return 1;
    }
    LOG_DONE();

    LOG_INFO("WiFi Firmware Version: {}", WiFi.firmwareVersion());

    LOG_DEBUG("Scanning Network...");
    int n = WiFi.scanNetworks();
    LOG_DEBUG("Found {} Networks", n);

    bool found = false;
    for (int i = 0; i < n; i++) {
        String ssid = WiFi.SSID(i);
        LOG_DEBUG("    {}: {} ({}dBm)", i, ssid, WiFi.RSSI(i));
        if (ssid == String(m_net_ssid)) {
            found = true;
            LOG_DEBUG("Found Target Network");
        }
    }

    if (!found) {
        LOG_ERROR("Target SSID Not Found: '{}'", m_net_ssid);
        return 2;
    }

    LOG_INFO("Connecting to {}", m_net_ssid);

    int status = WiFi.begin(m_net_ssid, m_net_pwd);
    delay(50);

    if (status == WL_CONNECT_FAILED) {
        LOG_ERROR("Cannot connect to target WiFi");
        return 3;
    }

    int ATTEMPTS = 40;
    while (WiFi.status() != WL_CONNECTED && ATTEMPTS--) {
        delay(500);
        LOG_DEBUG("Connecting Status: {}", WiFi.status());
    }

    status = WiFi.status();
    LOG_INFO("Connecting Status Finally: {}", status);

    switch (status) {
    case WL_CONNECTED:
        while (WiFi.localIP()[0] == 0) delay(100);
        udp.begin(m_udp_port);
        LOG_INFO("Connected. IP: {}, UDP Port: {}", WiFi.localIP(), m_udp_port);
        return 0;
    case WL_NO_SSID_AVAIL:
        LOG_ERROR("Cannot Find SSID");
        return 4;
    case WL_CONNECT_FAILED:
        LOG_ERROR("Password Wrong");
        return 5;
    case WL_IDLE_STATUS:
        LOG_WARN("Idle Status");
        return 6;
    case WL_DISCONNECTED:
        LOG_WARN("WL_DISCONNECTED");
        return 7;
    default:
        LOG_FATAL("UNKNOWING STATUS");
        return 8;
    }
}