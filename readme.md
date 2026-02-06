# AIPS

## PIN Usage
- Master Boards

| PIN No. | Function           | Connect to                            |
| ------- | ------------------ | ------------------------------------- |
| D0      | Serial Rx          | None                                  |
| D1      | Serial Tx          | None                                  |
| D2      | Motor Direction    | Left Motor Driver Direction           |
| D3      | Motor Enable (PWM) | Left Motor Driver Enable              |
| D4      | Motor Direction    | Right Motor Driver Direction          |
| D5      | Motor Enable (PWM) | Right Motor Driver Enable             |
| D6      | Motor Encoder INT  | Left Motor Encoder A                  |
| D7      | Motor Encoder INT  | Left Motor Encoder B                  |
| D9      | Motor Encoder INT  | Right Motor Encoder A                 |
| D8      | Motor Encoder INT  | Right Motor Encoder B                 |
| D10     | SPI SS             | Slave Board SS                        |
| D11     | SPI MOSI           | Slave Board MISO                      |
| D12     | SPI MISO           | Slave Board MOSI                      |
| D13     | SPI SCK            | Slave Board SCK                       |
| A0      | None               | None                                  |
| A1      | None               | None                                  |
| A2      | None               | None                                  |
| A3      | None               | None                                  |
| A4      | IIC DTA            | Modulinos(IMU, Buttons, Knob, Buzzer) |
| A5      | IIC SLC            | Modulinos(IMU, Buttons, Knob, Buzzer) |

- Slave Boards

| PIN No. | Function   | Connect to   |
| ------- | ---------- | ------------ |
| D0      | Serial Rx  | None         |
| D1      | Serial Tx  | None         |
| D2      | IR Reading | IR Reading 1 |
| D3      | IR Reading | IR Reading 2 |
| D4      | IR Reading | IR Reading 3 |
| D5      | IR Reading | IR Reading 4 |
| D6      | Servo PWM  | Servo        |
| D7      | IR Reading | IR Reading 5 |
| D9      | OLED DC    | OLED DC      |
| D8      | OLED RST   | OLED RS      |
| D10     | OLED CS    | OLED CS      |
| D11     | SPI MOSI   | OLED MISO    |
| D12     | SPI MISO   | OLED MOSI    |
| D13     | SPI SCK    | OLED SCK     |
| A0      | IR Reading | IR Reading 6 |
| A1      | IR Reading | IR Reading 7 |
| A2      | IR Reading | IR Reading 8 |
| A3      | IR Reading | IR Reading 9 |
| A4      | IIC DTA    | Modulinos    |
| A5      | IIC SLC    | Modulinos    |

## PIN Layout
### OLED 1362
| PIN No. | Symbol | Function          | Connect to |
| ------- | ------ | ----------------- | ---------- |
| 1       | GND    | gound             | GND        |
| 2       | VCC    | 1.65 V - 5.5 V    | 5V         |
| 3       | D0     | (SPI) SCLK        | D13        |
| 4       | D1     | (SPI) MOSI / SDIN | D11        |
| 5       | RES    | RESET             | D9         |
| 6       | D/C    | Data or Control   | D12        |
| 7       | CS     | Chip Select       | D10        |

### OLED 1306
| PIN No. | Symbol | Function   | Connect to |
| ------- | ------ | ---------- | ---------- |
| 1       | VCC    | 7 V - 15 V | 5V         |
| 2       | GND    | gound      | GND        |
| 3       | SCL    | IIC Clock  | A5 / SCL   |
| 4       | SDA    | IIC Data   | A4 / SDA   |

### Pololu Motor 1
| PIN Color | Function                   | Connect to |
| --------- | -------------------------- | ---------- |
| Red       | Motor Power +              | Ext ~12V   |
| Black     | Motor Power -              | Ext ~12V   |
| Green     | Encoder GND                | GND        |
| Blue      | Encoder VCC (3.5 V - 20 V) | 5V         |
| Yellow    | Encoder A                  | D2         |
| White     | Encoder B                  | D3         |

### Pololu Motor 2
| PIN Color | Function                   | Connect to |
| --------- | -------------------------- | ---------- |
| Red       | Motor Power +              | Ext ~12V   |
| Black     | Motor Power -              | Ext ~12V   |
| Green     | Encoder GND                | GND        |
| Blue      | Encoder VCC (3.5 V - 20 V) | 5V         |
| Yellow    | Encoder A                  | D7         |
| White     | Encoder B                  | D8         |
