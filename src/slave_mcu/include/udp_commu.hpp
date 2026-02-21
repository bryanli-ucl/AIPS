#pragma once

#include <WiFiUdp.h>

extern WiFiUDP udp;

class WifiCommu {
    public:
    WifiCommu(char* net_ssid, char* net_pwd, uint16_t udp_port);

    int8_t begin();

    private:
    char* m_net_ssid    = "BD4B Hyperoptic 1Gb Fibre 2.4Ghz";
    char* m_net_pwd     = "3R9gfN4up9ar";
    uint16_t m_udp_port = 9999;
};