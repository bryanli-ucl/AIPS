#include <Arduino.h>

#include "literals.hpp"
#include "peripherals.hpp"
#include "task_scheduler.hpp"

using namespace ::peripherals;
using namespace ::literals;

TaskScheduler scheduler{};

static int8_t dists[91]{};
static int8_t servo_angle{};

static float target_yaw{};
static float target_speed{};
static uint16_t item_type{};

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

        scheduler.add(500, []() { // send to pc
            robot_to_PC_wifi_data_t data;

            data.item_type = item_type;

            memcpy(data.dists, dists, sizeof(dists));
            data.degrees = servo_angle;

            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write((uint8_t*)&data, sizeof(data));
            udp.endPacket();

            LOG_INFO("UDP Sent: item_type={}, first_dist={}", data.item_type, data.dists[0]);

        },
        "udp send to pc");

        scheduler.add(30, []() { // UDP
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

            target_speed = module_vec(data.target_vel);
            target_yaw   = atan2(data.target_vel.x, data.target_vel.y);

            LOG_TRACE("Received Contents: {}, {}", data.target_vel.x, data.target_vel.y);
            if (module_vec_sq(data.target_vel)) {
                LOG_TRACE("Received Contents: {}, {}", data.target_vel.x, data.target_vel.y);
                buzzer.tone(440, 20);
            }

            iic_commu::slave2master_data.target_vel = target_speed;
            iic_commu::slave2master_data.target_yaw = target_yaw;
        },
        "Process UDP");

        scheduler.add(300, []() { // IR
            static std::array<uint16_t, IR_CONUT> sensor_values;
            qtr.read(sensor_values.data(), QTRReadMode::On);

            for (auto& x : sensor_values) {
                Serial.print(x);
                Serial.print(' ');
            }
            Serial.println();

            for (auto& x : sensor_values) {
                if (x > 250)
                    x = 1;
                else
                    x = 0;
            }

            item_type = 0;
            for (int i = 0; i < sensor_values.size(); i++) {
                item_type |= sensor_values[i] << i;
            }
            LOG_TRACE("{b}", item_type);

            // Prepare Data for Master requests
            auto& data = iic_commu::slave2master_data;

            data.target_vel = target_speed;
            data.target_yaw = target_yaw;
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

            // LOG_INFO("DATA.value1: {}", data.value1);
            // LOG_INFO("DATA.value2: {}", data.value2);
            // LOG_INFO("DATA.value3: {}", data.value3);


        },
        "Process IIC");

        scheduler.add(-1, []() { // OLED 1306 Display
        },
        "OLED 1306 Display (LiDAR Graph)");

        scheduler.add(200, []() { // OLED 1362
            auto draw_left = [](U8G2& d) {
                d.setFont(u8g2_font_6x12_tf);

                static char buf[128] = {};

                memset(buf, '\0', sizeof(buf));
                sprintf(buf, "SPEED: %f", target_speed);
                d.drawStr(2, 14, buf);

                memset(buf, '\0', sizeof(buf));
                sprintf(buf, "ITEM: %d", item_type);
                d.drawStr(2, 30, buf);

                memset(buf, '\0', sizeof(buf));
                sprintf(buf, "%d.%d.%d.%d", WiFi.localIP()[0], WiFi.localIP()[1], WiFi.localIP()[2], WiFi.localIP()[3]);
                d.drawStr(2, 46, buf);
            };

            auto draw_mid = [](U8G2& d) {
                int cx = 128;
                int cy = 32;

                d.drawCircle(cx, cy, 30);
                d.drawDisc(cx, cy, 2);

                if (fabs(target_speed) > 0.1f) {
                    float rad = target_yaw;
                    float r   = 28;
                    int x     = cx + r * cos(rad);
                    int y     = cy - r * sin(rad);

                    d.drawLine(cx, cy, x, y);

                    float l = 8;
                    d.drawLine(x, y, x - l * cos(rad - HALF_PI / 2.f), y + l * sin(rad - HALF_PI / 2.f));
                    d.drawLine(x, y, x - l * cos(rad + HALF_PI / 2.f), y + l * sin(rad + HALF_PI / 2.f));
                }
            };

            auto draw_right = [](U8G2& d) {
                constexpr int16_t cx = 256 - 85 / 2;
                constexpr int16_t cy = 63;
                constexpr int16_t l  = 55;

                auto p2xy = [](int16_t r, int16_t theta) {
                    auto t = theta * DEG_TO_RAD;
                    return std::tuple<int16_t, int16_t>(
                    r * sinf(t),
                    r * cosf(t));
                };

                auto dist2disp = [=](int16_t d) -> float {
                    return constrain(log2f(d) * 6 - 6, 2, l);
                    // return constrain(map(d, 0, 800, 10, l), 10, l);
                };

                for (auto t : { -45, 0, 45, servo_angle - 45 }) {
                    auto [x, y] = p2xy(l, t);
                    d.drawLine(cx, cy, cx + x, cy - y);
                }

                for (const auto& r : { 15, 30, (int)(l) }) {
                    static constexpr float factor = 255. / 360.;
                    d.drawArc(cx, cy, r, 45 * factor, 135 * factor);
                }

                for (int deg = 0; deg <= 90; deg++) {
                    auto [x, y] = p2xy(dist2disp(dists[deg]), deg - 45);
                    d.drawPixel(cx + x, cy - y);
                }
            };

            auto& d = oled1362.get_disp();
            d.clearBuffer();

            draw_left(d);
            d.drawVLine(85, 0, 64);
            draw_mid(d);
            d.drawVLine(170, 0, 64);
            draw_right(d);

            d.sendBuffer();

        },
        "OLED 1362 Display ");

        scheduler.add(6, []() { // LiDAR
            static int8_t sign{ 1 };

            tfl.Set_Trigger(iic_addrs::LiDAR);

            // data reading
            int16_t val;
            if (tfl.getData(val, iic_addrs::LiDAR)) {
                dists[servo_angle] = val;
                // LOG_DEBUG("LiDAR Reading: {}cm", dists[servo_angle]);
            } else {
                LOG_ERROR_START("LiDAR Read Error: ");
                tfl.printStatus();
                Serial.println();
                tfl.Soft_Reset(iic_addrs::LiDAR);
                // return;
            }
            // dists[servo_angle] = 15;
            const auto& dist = dists[servo_angle];
            // servo ratation

            // LOG_INFO("Servo angle: {}", servo_angle);
            servo.write(servo_angle);
            if (servo_angle >= 90) sign = -1;
            if (servo_angle <= 0) sign = 1;
            servo_angle = (servo_angle + 1 * sign);
        },
        "Lidar Reading & Servo");

        scheduler.reset();
    }
}


void loop() {
    scheduler.tick(millis());
}