#include <Arduino.h>

#include "literals.hpp"
#include "peripherals.hpp"
#include "task_scheduler.hpp"

using namespace ::peripherals;
using namespace ::literals;

TaskScheduler scheduler{};

static int16_t dists[91]{};
static int16_t servo_angle{};

void setup() {

    { // logger (Serial)
        LOG_BEGIN();
        delay(300); // essential
    }

    { // board info
        LOG_SECTION("ARDUINO UNO R4 WIFI SLAVE BOARS");
        LOG_INFO("sizeof(master_data): {}", sizeof(master_to_slave_iic_data_t));
        LOG_INFO("sizeof(slave2master_data): {}", sizeof(slave_to_master_iic_data_t));
    }

    { // init peripherals
        LOG_SECTION("INITIALIZING PERIPHERALS");
        peripherals::begin();
    }

    LOG_SECTION("PROGRAM BEGIN");

    { // Scheuler Tasks

        scheduler.add(-1, []() { // UDP
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

            auto& data = iic_commu::slave2master_data;

            data.target_vel = 18.88f;
            data.target_yaw = 1145.14f;
        },
        "Process IR");

        scheduler.add(10000, []() { // Print CPU Usage
            scheduler.print_cpu_usage();
        },
        "Print CPU Usage");

        scheduler.add(2500, []() { // Buzzer
            // buzzer.tone(440, 100);
        },
        "Buzzer");

        scheduler.add(-1, []() { // IIC
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

        scheduler.add(100, []() { // OLED 1362 Display
            auto& disp = oled1306.get_disp();

            static int16_t prev_dists[91]{};

            // Rendering
            if (servo_angle & 0b111) {
                constexpr int16_t HEIGHT = 64;
                constexpr int16_t WIDTH  = 128;

                static int16_t prev_angle{ -1 };

                auto p2xy = [](int16_t r, int16_t theta) -> std::tuple<int16_t, int16_t> const {
                    return { (float)r * sinf((float)theta * DEG_TO_RAD), (float)r * cosf((float)theta * DEG_TO_RAD) };
                };

                auto dist2disp = [](int16_t d) -> float {
                    return constrain(log2f(d) * 8 - 10, 3, 60);
                };

                for (int deg = 0; deg <= 90; deg++) {
                    auto [x, y] = p2xy(dist2disp(prev_dists[deg]), deg - 45);
                    disp.drawPixel(WIDTH / 2 + x, HEIGHT - y, SSD1306_BLACK);
                }


                auto draw_line = [&disp, p2xy](int16_t t, uint16_t color) -> void {
                    auto [x, y] = p2xy(60, t);
                    disp.drawLine(WIDTH / 2, HEIGHT, WIDTH / 2 + x, HEIGHT - y, color);
                    return;
                };

                for (const auto& t : { -45, 0, 45 }) {
                    draw_line(t, SSD1306_WHITE);
                }

                if (prev_angle != -1) {
                    int16_t t = prev_angle - 45;
                    draw_line(t, SSD1306_BLACK);
                    for (const auto& r : { 15, 30, 45, 60 }) {
                        auto [x, y] = p2xy(r, t);
                        disp.drawPixel(WIDTH / 2 + x, HEIGHT - y, SSD1306_WHITE);
                    }
                }
                draw_line(servo_angle - 45, SSD1306_WHITE);

                for (int deg = 0; deg <= 90; deg++) {
                    auto [x, y] = p2xy(dist2disp(dists[deg]), deg - 45);
                    disp.drawPixel(WIDTH / 2 + x, HEIGHT - y, SSD1306_WHITE);
                }

                memcpy(prev_dists, dists, sizeof(dists));

                prev_angle = servo_angle;

                disp.display();
            }
        },
        "OLED 1306 Display");

        scheduler.add(1000, []() { // OLED 1362
            // oled1362.clear();
            // oled1362.disp_status("OLED 1362", "Display Demo", "hello!!!");
        },
        "OLED 1362 Display");

        scheduler.add(20, []() { // LiDAR
            static int8_t sign{ 1 };

            delayMicroseconds(300);

            // data reading
            if (tfl.getData(dists[servo_angle], iic_addrs::LiDAR)) {
                // LOG_DEBUG("LiDAR Reading: {}cm", dists[servo_angle]);
            } else {
                LOG_ERROR("LiDAR Read Error: ");
                tfl.printStatus();
                Serial.println();
                tfl.Soft_Reset(iic_addrs::LiDAR);
                return;
            }
            const auto& dist = dists[servo_angle];
            // servo ratation

            // LOG_INFO("Servo angle: {}", servo_angle);
            servo.write(servo_angle);
            if (servo_angle >= 90) sign = -1;
            if (servo_angle <= 0) sign = 1;
            servo_angle = (servo_angle + 1 * sign);
        },
        "Lidar & Display");

        scheduler.reset();
    }
}


void loop() {
    scheduler.tick(millis());
}