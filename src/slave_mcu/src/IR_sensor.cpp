#include <Arduino.h>
#include <QTRSensors.h>

QTRSensors qtr;

const uint8_t SensorCount = 9;
const uint16_t BlackThreshold = 500;
uint16_t sensorValues[SensorCount];

void setup() {
    // configure the sensors
    qtr.setTypeRC();
    qtr.setSensorPins((const uint8_t[]){ 2, 3, 4, 5, 7, A0, A1, A2, A3 }, SensorCount);
    qtr.setEmitterPins(2, 3);

    delay(500);
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH); // turn on Arduino's LED to indicate we are in calibration mode

    // 2.5 ms RC read timeout (default) * 10 reads per calibrate() call
    // = ~25 ms per calibrate() call.
    // Call calibrate() 400 times to make calibration take about 10 seconds.
    for (uint16_t i = 0; i < 40; i++) {
        qtr.calibrate();
    }
    digitalWrite(LED_BUILTIN, LOW); // turn off Arduino's LED to indicate we are through with calibration

    // print the calibration minimum values measured when emitters were on
    Serial.begin(115200);
    for (uint8_t i = 0; i < SensorCount; i++) {
        Serial.print(qtr.calibrationOn.minimum[i]);
        Serial.print(' ');
    }
    Serial.println();

    // print the calibration maximum values measured when emitters were on
    for (uint8_t i = 0; i < SensorCount; i++) {
        Serial.print(qtr.calibrationOn.maximum[i]);
        Serial.print(' ');
    }
    Serial.println();
    Serial.println();

    qtr.setTimeout(9999);

    delay(1000);
}

void loop() {
    // Calibrated values are in the range 0-1000.
    // Lower values usually mean white (higher reflectance),
    // higher values usually mean black (lower reflectance).
    qtr.readCalibrated(sensorValues);

    for (uint8_t i = 0; i < SensorCount; i++) {
        const char* color = sensorValues[i] > BlackThreshold ? "BLACK" : "WHITE";
        Serial.print("S");
        Serial.print(i);
        Serial.print(":");
        Serial.print(color);
        Serial.print("(");
        Serial.print(sensorValues[i]);
        Serial.print(")\t");
    }
    Serial.println();

    delay(250);
}
