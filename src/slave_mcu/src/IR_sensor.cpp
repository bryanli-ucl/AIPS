#include <Arduino.h>
#include <QTRSensors.h>

QTRSensors qtr;

const uint8_t SensorCount = 9;
uint16_t sensorValues[SensorCount];
<<<<<<< HEAD
const uint8_t sensorPins[SensorCount] = {2, 3, 4, 5, 7, A0, A1, A2, A3};

void setup()
{
    Serial.begin(115200);

    qtr.setTypeRC();
    qtr.setSensorPins(sensorPins, SensorCount);

    delay(500);
}

void loop()
{
    qtr.read(sensorValues);

    for (uint8_t i = 0; i < SensorCount; i++)
    {
        Serial.print(sensorValues[i]);
        Serial.print('\t');
    }

    uint16_t position = qtr.readLineBlack(sensorValues);
    Serial.print("  Position=");
    Serial.print(position);
    
    Serial.println();

    delay(200);
=======

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
    // read calibrated sensor values and obtain a measure of the line position
    // from 0 to 5000 (for a white line, use readLineWhite() instead)
    // uint16_t position = qtr.readLineBlack(sensorValues);
    qtr.read(sensorValues);

    // print the sensor values as numbers from 0 to 1000, where 0 means maximum
    // reflectance and 1000 means minimum reflectance, followed by the line
    // position
    for (uint8_t i = 0; i < SensorCount; i++) {
        Serial.print(sensorValues[i]);
        Serial.print('\t');
    }
    Seri

    delay(250);
>>>>>>> 419f8437b3e2710f562b4628ce7298e96d78ea1f
}