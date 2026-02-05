# AIPS

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
