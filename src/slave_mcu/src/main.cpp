#include <Arduino.h>
#include <Wire.h>

#include "literals.hpp"
#include "peripherals.hpp"

using namespace ::peripherals;
using namespace ::literals;

void setup() {

    peripherals::begin();
}

void loop() {

    auto& data = iic_commu::master_data;

    if (data.is_new_data) {
        data.is_new_data = false;

        LOG_INFO("DATA.value1: {}", data.value1);
        LOG_INFO("DATA.value2: {}", data.value2);
        LOG_INFO("DATA.value3: {}", data.value3);
    }
}