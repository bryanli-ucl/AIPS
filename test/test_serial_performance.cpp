#include <Arduino.h>

#include "dev/serial_logger.hpp"

#include <unity.h>

void test_serial_println_performance(void) {
    Serial.println("\n=== Testing Serial.println Performance ===");

    unsigned long t, f, g;

    // 预热
    Serial.println("Warming up...");
    for (int i = 0; i < 5; i++) {
        Serial.println("[INFO ] [src/main.cpp:42] HELLO");
    }
    delay(100);

    // 测试单次调用
    Serial.println("\nTesting single call...");
    t = micros();
    Serial.println("[INFO ] [src/main.cpp:42] HELLO");
    f = micros() - t;

    Serial.print("Single call time: ");
    Serial.print(f);
    Serial.println(" us");

    // 测试 10 次调用
    Serial.println("\nTesting 10 calls...");
    t = micros();
    for (int i = 0; i < 10; i++) {
        Serial.println("[INFO ] [src/main.cpp:42] HELLO");
    }
    g = micros() - t;

    float avg_time = (g - f) / 10.0;

    Serial.print("10 calls total time: ");
    Serial.print(g);
    Serial.println(" us");
    Serial.print("Average time per call: ");
    Serial.print(avg_time, 2);
    Serial.println(" us");

    Serial.println("=== Performance Test Complete ===\n");

    // 断言
    TEST_ASSERT_LESS_THAN(1, f);
    TEST_ASSERT_LESS_THAN(1, avg_time);
}

void test_multiple_runs_statistics(void) {
    Serial.println("\n=== Running Statistical Analysis (100 samples) ===");

    const int samples = 100;
    unsigned long times[samples];
    unsigned long sum      = 0;
    unsigned long min_time = ULONG_MAX;
    unsigned long max_time = 0;

    // 收集样本
    for (int i = 0; i < samples; i++) {
        unsigned long t = micros();
        Serial.println("[INFO ] [src/main.cpp:42] HELLO");
        times[i] = micros() - t;

        sum += times[i];
        if (times[i] < min_time) min_time = times[i];
        if (times[i] > max_time) max_time = times[i];
    }

    float avg = sum / (float)samples;

    Serial.print("Samples: ");
    Serial.println(samples);
    Serial.print("Average: ");
    Serial.print(avg, 2);
    Serial.println(" us");
    Serial.print("Min: ");
    Serial.print(min_time);
    Serial.println(" us");
    Serial.print("Max: ");
    Serial.print(max_time);
    Serial.println(" us");
    Serial.println("=== Statistical Analysis Complete ===\n");

    TEST_ASSERT_LESS_THAN(2, max_time);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }

    delay(2000);

    Serial.println("\n\n");
    Serial.println("╔════════════════════════════════════════╗");
    Serial.println("║  Serial Performance Test Suite        ║");
    Serial.println("║  Arduino UNO R4 WiFi                  ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println();

    UNITY_BEGIN();

    RUN_TEST(test_serial_println_performance);
    RUN_TEST(test_multiple_runs_statistics);

    UNITY_END();
}

void loop() {
}