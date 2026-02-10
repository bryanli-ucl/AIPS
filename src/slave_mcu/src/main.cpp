#include <Arduino.h>
#include <Wire.h>

#include "dev/peripherals.hpp"
#include "literals.hpp"

using namespace ::peripherals;
using namespace ::literals;

void setup() {

    peripherals::begin();

    LOG_INFO("master_iic_data_t size: {}", sizeof(iic_commu::master_data));
}

void loop() {

    auto& data = iic_commu::master_data;

    static int test_counter = 0;
    if (test_counter++ == 0) {
        data.is_new_data = true;
        data.value1      = 99;
        data.value2      = 88;
        data.value3      = 77;
    }

    if (data.is_new_data || true) {
        data.is_new_data = false;

        LOG_INFO("DATA.value1: {}", data.value1);
        LOG_INFO("DATA.value2: {}", data.value2);
        LOG_INFO("DATA.value3: {}", data.value3);
    }

    delay(500);
}