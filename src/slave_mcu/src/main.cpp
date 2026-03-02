#include <Arduino.h>

#include "literals.hpp"
#include "peripherals.hpp"
#include "task_scheduler.hpp"

using namespace ::peripherals;
using namespace ::literals;

TaskScheduler scheduler{};

void setup() {

    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
    }

    { // board info
        LOG_SECTION("ARDUINO UNO R4 WIFI SLAVE BOARS");
        LOG_INFO("sizeof(master_data): {}", sizeof(master_to_slave_iic_data_t));
        LOG_INFO("sizeof(slave_data): {}", sizeof(slave_to_master_iic_data_t));
    }

    { // init peripherals
        LOG_SECTION("INITIALIZING PERIPHERALS");
        peripherals::begin();
    }

    LOG_SECTION("PROGRAM BEGIN");

    { // Scheuler Tasks

        scheduler.add(50, []() { // UDP
            // Check and Recieve data
            static uint8_t buf[256] = {};

            int pack_len = udp.parsePacket();
            if (pack_len == 0) return;

            LOG_TRACE("Received From {}:{}, Packet Size: {}", udp.remoteIP(), udp.remotePort(), pack_len);

            int len = udp.read(buf, sizeof(buf));

            if (len != 2 * sizeof(float)) {
                LOG_WARN("Error Length UDP pack");
                return;
            }

            PC_to_robot_wifi_data_t data{
                .target_vel = {
                .x = *((float*)(&buf[0])),
                .y = *((float*)(&buf[4])),
                .z = 0.f,
                }
            };

            // Process Recieved Data

            if (module_vec_sq(data.target_vel)) {
                LOG_DEBUG("Received Contents: {}, {}", data.target_vel.x, data.target_vel.y);
                buzzer.tone(440, 20);
            }
        },
        "Process UDP");

        scheduler.add(50, []() { // IR
            static uint16_t sensor_values[IR_CONUT] = {};

            uint16_t pos = qtr.readLineBlack(sensor_values);
            // LOG_DEBUG("Position: {}", pos);
        },
        "Process IR");

        scheduler.add(1000, []() { // Print CPU Usage
            scheduler.print_cpu_usage();
        },
        "Print CPU Usage");

        scheduler.add(2500, []() { // Buzzer
            // buzzer.tone(440, 100);
        },
        "Buzzer Test");

        scheduler.add(25, []() { // IIC
            auto& data = iic_commu::master_data;
            if (!data.is_new_data)
                return -1;
            else
                data.is_new_data = false;

            LOG_INFO("DATA.value1: {}", data.value1);
            LOG_INFO("DATA.value2: {}", data.value2);
            LOG_INFO("DATA.value3: {}", data.value3);

        },
        "Process IIC");

        scheduler.reset();
    }
}

void loop() {
    scheduler.tick(millis());
}