#pragma once

#include <stdint.h>

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
