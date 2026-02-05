# AIPS
## Pin Usage

| PIN No. | Function            | Connect to                   |
| ------- | ------------------- | ---------------------------- |
| D0      | Serial Rx           | None                         |
| D1      | Serial Tx           | None                         |
| D2      | Motor Direction     | Left Motor Driver Direction  |
| D3      | Motor Enable  (PWM) | Left Motor Driver Enable     |
| D4      | Motor Direction     | Right Motor Driver Direction |
| D5      | Motor Enable  (PWM) | Right Motor Driver Enable    |
| D6      | Servo (PWN)         | Servo Control Pin            |
| D7      | None                | None                         |
| D8      | OLED DC             | OLED 1362 D/C                |
| D9      | OLED RST            | OLED 1362 Reset              |
| D10     | OLED CS             | OLED 1362 Chip Select        |
| D11     | SPI MOSI            | OLED 1362 MOSI PIN           |
| D12     | SPI MISO            | None                         |
| D13     | SPI SCK             | OLED 1362 SCK PIN            |
| A0      | Encoder Interrupt   | Left Motor Encoder A         |
| A1      | Encoder Interrupt   | Left Motor Encoder B         |
| A2      | Encoder Interrupt   | Right Motor Encoder A        |
| A3      | Encoder Interrupt   | Right Motor Encoder B        |
| A4      | IIC DTA             | Modulinos, OLED1306, LiDAR   |
| A5      | IIC SLC             | Modulinos, OLED1306, LiDAR   |

## pin layout
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

### Motor driver
| PIN No. | Symbol | Function             | Connect to |
| ------- | ------ | -------------------- | ---------- |
| 1       | DIR_L  | direction of motor L | UNKNOW     |
| 2       | EN_L   | speed of motor L     | UNKNOW     |
| 3       | DIR_R  | direction of motor L | 5V         |
| 4       | EN_R   | speed of motor L     | GND        |
| 3       | VCC    | 12v                  | bat.       |
| 4       | GND    | ground               | GND        |

### QTR-HD-09RC Reflectance Sensor Array
| Pin Symbol | Function         | Connect to |
| ---------- | ---------------- | ---------- |
| VCC        | 2.9 V to 5.5 V   | 5V         |
| GND        | ground           | GND        |
| CTRL       | enable 2,4,6,8   | UNKNOW     |
| CTRL ODD   | enable 1,3,5,7,9 | UNKNOW     |



