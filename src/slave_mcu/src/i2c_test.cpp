#include <Arduino.h>
#include <Wire.h>

#define MASTER
// #define SLAVE

#ifdef SLAVE

void receiveEvent(int howMany);

char received_buf[64] = {};

int16_t sp = -1;


void setup() {
    Serial.begin(115200);

    Wire1.begin(8); // 从机地址 = 8
    Wire1.onReceive(receiveEvent);

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("Slave ready");
}

void loop() {
    if (sp != -1) {
        Serial.println(received_buf);
        sp = -1;
        memset(received_buf, 0, sizeof(received_buf));
    }
}

void receiveEvent(int howMany) {
    digitalWrite(LED_BUILTIN, HIGH);
    sp = 0;
    while (Wire1.available()) {
        char c             = Wire1.read();
        received_buf[sp++] = c;
    }
    digitalWrite(LED_BUILTIN, LOW);
}

#endif

#ifdef MASTER

#    include <Modulino.h>

ModulinoKnob knob;

void setup() {
    Serial.begin(115200);
    Wire1.begin(); // 主机模式

    Modulino.begin();
    knob.begin();

    Serial.println("Master ready");
}

void loop() {
    Serial.print(knob.get());
    Serial.println(" Send data: ");

    Wire1.beginTransmission(8); // 发给地址8
    Wire1.write('A');           // 发送字符A
    Wire1.endTransmission();

    delay(1000);
}

#endif