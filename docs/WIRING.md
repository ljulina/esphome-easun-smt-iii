# Wiring Notes

> [!WARNING]
> This wiring is documented for **Solar Plug-RWB1 ver. -06R** only. Other Solar Plug / inverter control board versions may use a different RJ45 pinout. Verify every pin with your own hardware before connecting anything.

> [!WARNING]
> No warranty is provided. Incorrect wiring can damage the inverter, ESP32, RS232 converter, power supply, connected equipment, or create an electrical hazard. Use this information at your own risk.

## Tested Hardware

- EASUN SMT-III inverter with Solar Plug-RWB1 ver. -06R
- ESP32 DevKit
- MAX3232 RS232 to TTL converter
- DC-DC step-down converter, 12 V to 3.3 V
- RJ45 cable

## Assembled Unit

![Assembled ESP32, MAX3232, and power supply in a 3D printed enclosure](../assets/photos/esp32-rs232-power-box-assembled.jpeg)

Assembled example for **Solar Plug-RWB1 ver. -06R**.

## Wiring Photo

![ESP32, MAX3232, and power supply wiring detail](../assets/photos/esp32-rs232-power-box-wiring.jpeg)

Wiring detail for **Solar Plug-RWB1 ver. -06R**.

## Connection Diagram

```text
WARNING: Solar Plug-RWB1 ver. -06R only

                RS232                UART-TTL
+------------+          +---------+              +----------+
|  SMT-III   |          | MAX3232 |              |  ESP32   |
|            |          |         |              |          |
|          3 | TX ----->| RX      | TX TTL ----->| RX GPIO3 |
|          6 | RX <-----| TX      | RX TTL <-----| TX GPIO1 |
|          5 | GND -----| GND     | GND ---------| GND      |
|            |          |         | VCC (+)      | 3.3V     |
|          2 | 12V --+  +---------+              +----------+
+------------+       |
                     v
                    IN+
            Step-down 12V -> 3.3V
                    VO+
                     |
                     +-- ESP32 3V3
                     +-- MAX3232 VCC (+)
```

## RJ45 Pinout

For **Solar Plug-RWB1 ver. -06R**:

| RJ45 pin | T-568B color | Description | MAX3232 pin | ESP32 pin | DC-DC power supply |
| --- | --- | --- | --- | --- | --- |
| 2 | Orange | VCC +12 V | - | - | IN+ |
| 3 | White-Green | TX | RX | RX GPIO3 | - |
| 5 | White-Blue | GND | GND | GND | GND |
| 6 | Green | RX | TX | TX GPIO1 | - |

## UART Settings

The example ESPHome configuration uses:

- Baud rate: `2400`
- ESP32 TX: `GPIO1`
- ESP32 RX: `GPIO3`
- Logger UART disabled with `logger.baud_rate: 0`

## Notes Before Power-Up

1. Confirm the Solar Plug board revision.
2. Confirm RJ45 pin numbers and cable colors with a multimeter.
3. Confirm the DC-DC converter output is 3.3 V before connecting the ESP32 or MAX3232.
4. Confirm all grounds are connected together.
5. Confirm RS232 side and TTL side of the MAX3232 are not swapped.
6. Keep the first power-up supervised.

## Images

- Assembled unit: [`assets/photos/esp32-rs232-power-box-assembled.jpeg`](../assets/photos/esp32-rs232-power-box-assembled.jpeg)
- Wiring photo: [`assets/photos/esp32-rs232-power-box-wiring.jpeg`](../assets/photos/esp32-rs232-power-box-wiring.jpeg)
- RJ45 diagram: [`assets/diagrams/rj45-solar-plug-rwb1-06r.png`](../assets/diagrams/rj45-solar-plug-rwb1-06r.png)
- Connection diagram: ASCII block in this document
- 3D enclosure render: [`hardware/schematics/esp32-rs232-power-box.png`](../hardware/schematics/esp32-rs232-power-box.png)
- More photos: [`assets/photos/`](../assets/photos/)
