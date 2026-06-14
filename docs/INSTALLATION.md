# ESPHome and Home Assistant Installation

> [!WARNING]
> This project is a community-made guide and external component. It is not affiliated with or supported by EASUN. Use it at your own risk.

## Prerequisites

- Home Assistant with ESPHome installed.
- ESP32 board supported by ESPHome.
- Working Wi-Fi credentials in ESPHome `secrets.yaml`.
- Wiring completed and verified according to [`WIRING.md`](WIRING.md).

## Files To Copy

Copy the external component into your ESPHome configuration directory:

```text
components/easun_smt_iii/
```

Use this example as a starting point:

```text
examples/easun3-with-component.yaml
```

If your ESPHome configuration directory already contains a `components` directory, copy only the `easun_smt_iii` folder into it.

## Example Configuration

The example configuration uses a local external component:

```yaml
external_components:
  - source:
      type: local
      path: components
```

Alternatively, install the component directly from GitHub (use a release tag for a fixed version):

```yaml
external_components:
  - source:
      type: git
      url: https://github.com/ljulina/esphome-easun-smt-iii
      ref: v0.1.0
    components: [easun_smt_iii]
```

The example YAML also sets the ESPHome project version shown in Home Assistant:

```yaml
esphome:
  project:
    name: ljulina.esphome-easun-smt-iii
    version: "0.1.0"
```

The UART configuration is:

```yaml
uart:
  - id: uart_0
    baud_rate: 2400
    tx_pin: GPIO1
    rx_pin: GPIO3
```

## Secrets

The example YAML expects these ESPHome secrets:

```yaml
wifi_ssid: "your-wifi"
wifi_password: "your-password"
ota_password: "your-ota-password"
api_encryption_key: "your-home-assistant-api-key"
```

Do not commit your own `secrets.yaml` to GitHub.

## Flashing

1. Open ESPHome in Home Assistant.
2. Add a new ESP32 device or import the example YAML.
3. Copy the external component directory into the ESPHome configuration directory.
4. Validate the YAML.
5. Compile and flash the ESP32.
6. Add the ESPHome device to Home Assistant.

## Time Synchronization

The example includes an SNTP time source and a button for synchronizing the inverter date and time from Home Assistant:

```yaml
time:
  - platform: sntp
    id: ha_time
    timezone: Europe/Prague
```

Change the timezone to match your installation.

## Troubleshooting

- If the ESP32 logs are noisy or UART communication does not work, confirm `logger.baud_rate: 0`.
- If there is no response from the inverter, re-check TX/RX crossing between the Solar Plug, MAX3232, and ESP32.
- If the ESP32 resets or behaves randomly, verify the step-down converter output and current capacity.
- If values are missing, increase response timeout or check the RS232 signal wiring.
