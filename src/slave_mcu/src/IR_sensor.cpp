#include <Arduino.h>
#include <QTRSensors.h>

QTRSensors qtr;

const uint8_t SensorCount = 9;
uint16_t sensorValues[SensorCount];
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
}