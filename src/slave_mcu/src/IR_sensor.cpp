#include <Arduino.h>

const int irPins[9] = {2, 3, 4, 5, 7, A0, A1, A2, A3};

// true = 黑线白底
// false = 白线黑底
const bool LINE_IS_BLACK = true;

// true = 传感器检测到目标时输出 0
// false = 传感器检测到目标时输出 1
const bool SENSOR_ACTIVE_LOW = true;

// 从左到右的权重
const int weights[9] = {-4, -3, -2, -1, 0, 1, 2, 3, 4};

int sensorState[9];
int lastError = 0;

void readSensors() {
    for (int i = 0; i < 9; i++) {
        int raw = digitalRead(irPins[i]);

        // 先统一成“检测到反射目标/有效触发”为 1
        int detected = SENSOR_ACTIVE_LOW ? !raw : raw;

        // 如果是黑线白底，则“黑线”应该算触发
        // 如果是白线黑底，则反过来
        sensorState[i] = LINE_IS_BLACK ? detected : !detected;
    }
}

int calculateError() {
    long weightedSum = 0;
    int count = 0;

    for (int i = 0; i < 9; i++) {
        if (sensorState[i]) {
            weightedSum += weights[i];
            count++;
        }
    }

    // 一条线都没看到
    if (count == 0) {
        return lastError;
    }

    int error = weightedSum / count;
    lastError = error;
    return error;
}

void printSensors() {
    for (int i = 0; i < 9; i++) {
        Serial.print(sensorState[i]);
        Serial.print(" ");
    }
}

void setup() {
    Serial.begin(115200);

    for (int i = 0; i < 9; i++) {
        pinMode(irPins[i], INPUT);
    }
}

// 这里先不接电机，只输出循迹方向判断
void loop() {
    readSensors();
    int error = calculateError();

    printSensors();
    Serial.print(" | error = ");
    Serial.print(error);
    Serial.print(" | ");

    if (error == 0) {
        Serial.println("forward");
    }
    else if (error < 0) {
        Serial.println("turn left");
    }
    else {
        Serial.println("turn right");
    }

    delay(100);
}