#pragma once

#include <stddef.h>
#include <stdint.h>

namespace iic_addrs {

constexpr uint8_t SlaveMCU = 0x08;
constexpr uint8_t LiDAR    = 0x10;
constexpr uint8_t Motoron  = 0x10;

}; // namespace iic_addrs


#pragma pack(push, 1)

struct master_iic_data_t {
    uint16_t value1;
    int16_t value2;
    float value3;
    uint8_t check_sum : 7;
    uint8_t is_new_data : 1;
};


struct slave_iic_data_t {
    uint16_t value1;
    uint16_t value2;
    uint16_t value3;
    uint16_t value4;
    uint8_t check_sum : 7;
    uint8_t is_new_data : 1;
};

#pragma pack(pop)

template <size_t N>
constexpr size_t strlen_ce(const char (&)[N]) {
    return N - 1;
}