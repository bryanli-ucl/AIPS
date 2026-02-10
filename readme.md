# AIPS

## PIN Usage
- Master Boards

| PIN No. | Function                | Connect to                                   |
| ------- | ----------------------- | -------------------------------------------- |
| D0      | Serial Rx               | None                                         |
| D1      | Serial Tx               | None                                         |
| D2      | Motor Encoder INT       | Left Motor Encoder A                         |
| D3      | Motor Encoder INT       | Right Motor Encoder A                        |
| D4      | Motor Encoder Read      | Left Motor Encoder B                         |
| D5      | Motor Encoder Read      | Right Motor Encoder B                        |
| D6      | Covered By Motor Driver | Covered By Motor Driver                      |
| D7      | Covered By Motor Driver | Covered By Motor Driver                      |
| D9      | Covered By Motor Driver | Covered By Motor Driver                      |
| D8      | Covered By Motor Driver | Covered By Motor Driver                      |
| D10     | Covered By Motor Driver | Covered By Motor Driver                      |
| D11     | Covered By Motor Driver | Covered By Motor Driver                      |
| D12     | Covered By Motor Driver | Covered By Motor Driver                      |
| D13     | Covered By Motor Driver | Covered By Motor Driver                      |
| A0      | None                    | None                                         |
| A1      | None                    | None                                         |
| A2      | None                    | None                                         |
| A3      | None                    | None                                         |
| A4      | IIC DTA                 | Modulinos(IMU, Buzzer), Motoron, Slave Board |
| A5      | IIC SLC                 | Modulinos(IMU, Buzzer), Motoron, Slave Board |

- Slave Boards

| PIN No. | Function   | Connect to                     |
| ------- | ---------- | ------------------------------ |
| D0      | Serial Rx  | None                           |
| D1      | Serial Tx  | None                           |
| D2      | IR Reading | IR Reading 1                   |
| D3      | IR Reading | IR Reading 2                   |
| D4      | IR Reading | IR Reading 3                   |
| D5      | IR Reading | IR Reading 4                   |
| D6      | Servo PWM  | Servo                          |
| D7      | IR Reading | IR Reading 5                   |
| D8      | OLED RST   | OLED 1362 RS                   |
| D9      | OLED DC    | OLED 1362 DC                   |
| D10     | OLED CS    | OLED 1362 CS                   |
| D11     | SPI MOSI   | OLED 1362 MISO                 |
| D12     | SPI MISO   | OLED 1362 MOSI (None)          |
| D13     | SPI SCK    | OLED 1362 SCK                  |
| A0      | IR Reading | IR Reading 6                   |
| A1      | IR Reading | IR Reading 7                   |
| A2      | IR Reading | IR Reading 8                   |
| A3      | IR Reading | IR Reading 9                   |
| A4      | IIC DTA    | OLED 1306, LiDAR, Master Board |
| A5      | IIC SLC    | OLED 1306, LiDAR, Master Board |

## Component Details
### OLED 1362 SPI
| PIN No. | Symbol | Function          | Connect to |
| ------- | ------ | ----------------- | ---------- |
| 1       | GND    | gound             | GND        |
| 2       | VCC    | 1.65 V - 5.5 V    | 5V         |
| 3       | D0     | (SPI) SCLK        | SD13(SPI)  |
| 4       | D1     | (SPI) MOSI / SDIN | SD11(SPI)  |
| 5       | RES    | RESET             | SRST       |
| 6       | D/C    | Data or Control   | SD9        |
| 7       | CS     | Chip Select       | SD10       |

### OLED 1306 IIC
| PIN No. | Symbol | Function   | Connect to |
| ------- | ------ | ---------- | ---------- |
| 1       | VCC    | 7 V - 15 V | 5V         |
| 2       | GND    | gound      | GND        |
| 3       | SCL    | IIC Clock  | SA5 / SSCL |
| 4       | SDA    | IIC Data   | SA4 / SSDA |

### Pololu Motor 1
9.7:1 Metal Gearmotor 25Dx63L mm LP 12V with 48 CPR Encoder #4882
| PIN Color | Function                   | Connect to |
| --------- | -------------------------- | ---------- |
| Red       | Motor Power +              | Ext ~12V   |
| Black     | Motor Power -              | Ext ~12V   |
| Green     | Encoder GND                | GND        |
| Blue      | Encoder VCC (3.5 V - 20 V) | 5V         |
| Yellow    | Encoder A                  | MD2        |
| White     | Encoder B                  | MD4        |

### Pololu Motor 2
9.7:1 Metal Gearmotor 25Dx63L mm LP 12V with 48 CPR Encoder #4882
| PIN Color | Function                   | Connect to |
| --------- | -------------------------- | ---------- |
| Red       | Motor Power +              | Ext ~12V   |
| Black     | Motor Power -              | Ext ~12V   |
| Green     | Encoder GND                | GND        |
| Blue      | Encoder VCC (3.5 V - 20 V) | 5V         |
| Yellow    | Encoder A                  | MD3        |
| White     | Encoder B                  | MD5        |

### IR Sensors 
QTRX-HD-09RC Reflectance Sensor Array: 9-Channel, 4mm Pitch, RC Output, Low Current #4309
| PIN Color | Function                   | Connect to |
| --------- | -------------------------- | ---------- |
| Red       | Motor Power +              | Ext ~12V   |
| Black     | Motor Power -              | Ext ~12V   |
| Green     | Encoder GND                | GND        |
| Blue      | Encoder VCC (3.5 V - 20 V) | 5V         |
| Yellow    | Encoder A                  | MD3        |
| White     | Encoder B                  | MD5        |

### LiDAR 
TF-Luna LiDAR
| PIN No. | Symbol | Function     | Connect to |
| ------- | ------ | ------------ | ---------- |
| 1       | +5V    | Power Supply | 5V         |
| 2       | SDA    | IIC Data     | SA4 / SSDA |
| 3       | SCL    | IIC Clock    | SA5 / SSCL |
| 4       | GND    | gound        | GND        |
| 5       | None   | None         | None       |
| 6       | None   | None         | None       |
