#pragma once

#include <math.h>
#include <stddef.h>
#include <stdint.h>

namespace iic_addrs {

constexpr uint8_t SlaveMCU = 0x08;
constexpr uint8_t LiDAR    = 0x10;
constexpr uint8_t Motoron  = 0x19;

}; // namespace iic_addrs


#pragma pack(push, 1)

struct Vector {
    float x;
    float y;
    float z;
};

namespace udp_send {

struct recieve_data_t {
};

}; // namespace udp_send

struct master_to_slave_iic_data_t {
    uint16_t value1;
    int16_t value2;
    float value3;
    uint8_t check_sum : 7;
    uint8_t is_new_data : 1;
};

struct slave_to_master_iic_data_t {
    float target_vel;
    float target_yaw;
    uint8_t check_sum : 7;
    uint8_t is_new_data : 1;
};

struct robot_to_PC_wifi_data_t {
};

struct PC_to_robot_wifi_data_t {
    Vector target_vel;
};

#pragma pack(pop)

constexpr Vector normalize(Vector v) {
    float l = sqrt(v.x * v.x + v.y + v.y + v.z * v.z);
    return { v.x / l, v.y / l, v.z / l };
}

constexpr float module_vec(Vector v) {
    return sqrt(v.x * v.x + v.y + v.y + v.z * v.z);
}

constexpr float module_vec_sq(Vector v) {
    return v.x * v.x + v.y + v.y + v.z * v.z;
}

template <size_t N>
constexpr size_t strlen_ce(const char (&)[N]) {
    return N - 1;
}