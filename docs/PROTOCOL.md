# Protocol Notes

> [!WARNING]
> These notes are based on observed communication with tested hardware. They are not an official EASUN protocol specification and may be incomplete, incorrect, or different for other inverter firmware versions or Solar Plug board revisions.

> [!CAUTION]
> Commands that change inverter settings can affect power output, battery charging, battery protection, grid behavior, and safety-related operation. Do not send write commands unless you understand the setting and have verified it on your own hardware.

## Purpose

This document summarizes the observed EASUN SMT-III serial protocol behavior used by this ESPHome component.

The field names below are a mix of confirmed values and best-effort observations. Fields marked as `Unknown` or `Tentative` should not be treated as verified.

## How The Protocol Was Identified

The protocol was identified by observing communication between the inverter and its Solar Plug module, then testing equivalent requests from an ESP32 through an RS232 to TTL converter.

General workflow:

1. Capture request and response frames from the existing Solar Plug communication.
2. Compare repeated frames under different inverter states.
3. Identify stable command names, payload positions, units, scaling, terminators, and checksum behavior.
4. Reproduce selected requests from ESPHome.
5. Compare ESPHome responses with the captured Solar Plug communication.
6. Add decoded values to the ESPHome component only when the field meaning is reasonably clear.

## Transport

- Physical interface: RS232 on the inverter side.
- ESP32 interface: UART TTL through a MAX3232 RS232 to TTL converter.
- Baud rate used by the example configuration: `2400`.
- Example ESP32 UART pins: `GPIO1` TX and `GPIO3` RX.
- Frame terminator observed in captures: carriage return, `0x0D`.

See [`WIRING.md`](WIRING.md) before connecting hardware.

## Frame Format

Read commands are ASCII-like command names followed by carriage return.

Example request:

```text
RAW: HOP.
HEX: 48 4F 50 0D
```

In the notes above, the trailing `.` represents the carriage return byte (`0x0D`) as displayed by the serial logging tool.

Responses usually start with `(`, contain space-separated ASCII fields, and end with carriage return.

Example response:

```text
RAW: (230.3 50.0 00161 00022 002 420 06200 001.7 00002.
HEX: 28 32 33 30 2E 33 20 35 30 2E 30 20 30 30 31 36 31 20 30 30 30 32 32 20 30 30 32 20 34 32 30 20 30 36 32 30 30 20 30 30 31 2E 37 20 30 30 30 30 32 0D
```

The checksum implementation used by the ESPHome component is in:

```text
components/easun_smt_iii/easun_checksum.cpp
components/easun_smt_iii/easun_checksum.h
```

## ACK And NAK

Write commands appear to return either an ACK or NAK style response.

```text
ACK raw: (ACK9 .
ACK hex: 28 41 43 4B 39 20 0D

NAK raw: (NAKss.
NAK hex: 28 4E 41 4B 73 73 0D
```

If a write command returns `NAK`, it is usually safer to re-read the current setting before trying again.

## Polling Model

The ESPHome component uses a queue-based polling model:

1. A command is selected for polling.
2. The command is written to UART.
3. The component waits for a response or timeout.
4. The response is parsed and published to ESPHome sensors.
5. The next command is sent after the configured delay / throttle.

This avoids multiple simultaneous UART requests and keeps communication predictable.

## Read Commands

### `HOP` - Output Values

Example:

```text
RAW: HOP.
RAW: (230.3 50.0 00161 00022 002 420 06200 001.7 00002.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Output Voltage | V |
| 1 | Output Frequency | Hz |
| 2 | Output Apparent Power | VA, for example `00161` = 161 VA |
| 3 | Output Active Power | W, for example `00022` = 22 W |
| 4 | Output Load Percent | % |
| 5 | DC Bus Voltage | V, previously marked as unknown but observed as bus voltage |
| 6 | Rated Power | VA, for example `06200` = 6200 VA |
| 7 | Output Current | A, observed as real AC output current |
| 8 | Status | Bitmask / code, not fully decoded |

### `HGRID` - Grid Settings / Grid Limits

Example:

```text
RAW: HGRID.
RAW: (000.0 00.0 264 090 65 45 +00000 0 06200 10+00000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | AC Voltage | V |
| 1 | AC Frequency | Hz |
| 2 | High Point Of Mains | V |
| 3 | Low Point Of Mains | V |
| 4 | High Frequency Of Mains | Hz |
| 5 | Low Frequency Of Mains | Hz |
| 6 | Unknown | Signed value |
| 7 | Unknown | Possibly mode / flag |
| 8 | Rated Power | Tentative, `06200` = 6200 VA |
| 9 | Unknown | Composite field in observed response |

### `HBAT` - Battery Values

Example:

```text
RAW: HBAT.
RAW: (04 052.8 046 000 00000 372 110007200000 00000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Battery Number In Series | Count |
| 1 | Battery Voltage | V |
| 2 | Battery Capacity | % |
| 3 | Battery Charging Current | A, tentative |
| 4 | Battery Discharging Current | A, tentative |
| 5 | DC Bus Voltage | V |
| 6 | Unknown | Composite value / flags |
| 7 | Unknown | Flags / reserved |

### `HPV` - PV Values

Example:

```text
RAW: HPV.
RAW: (070.0 00.3 00022 00000.0 00000 2 070.0 027 08500.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | PV Voltage | V |
| 1 | PV Current | A |
| 2 | PV Power | W |
| 3 | Unknown | Possibly accumulated or secondary PV value |
| 4 | Unknown | Possibly accumulated or secondary PV value |
| 5 | PV Operating Mode | Tentative enum |
| 6 | PV Max Voltage | V |
| 7 | PV Temperature | Celsius, tentative |
| 8 | PV Max Power | W |

### `HTEMP` - Temperatures And Fan Values

Example:

```text
RAW: HTEMP.
RAW: (018 024 022 001 024 050 050 11000000000000000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | PV Temperature | Celsius |
| 1 | Inverter Temperature | Celsius |
| 2 | Boost Temperature | Celsius |
| 3 | Fan State / Fan Mode | Tentative enum |
| 4 | Ambient / Secondary Temperature | Tentative, Celsius |
| 5 | Fan 1 Speed | Unit unknown |
| 6 | Fan 2 Speed | Unit unknown |
| 7 | Unknown | Flags / composite value |

### `HGEN` - Energy Generation Counters

Example:

```text
RAW: HGEN.
RAW: (260417 18:24 04.794 0023.6 0023.6 000000028.1 000000000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Date | `YYMMDD` |
| 1 | Time | `HH:MM` |
| 2 | Daily Power Generation | Total PV, unit likely kWh |
| 3 | Monthly Power Generation | Total PV, unit likely kWh |
| 4 | Yearly Power Generation | Total PV, unit likely kWh |
| 5 | Total Power Generation | Total PV, unit likely kWh |
| 6 | Unknown | Flags / reserved |

### `HIMSG1` - Software Information

Example:

```text
RAW: HIMSG1.
RAW: (0020.04 20260108 12.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Software Version | Example: `0020.04` |
| 1 | Software Date | Example: `20260108` |
| 2 | Firmware Variant / Hardware ID / Protocol Revision | Tentative |

### `HSTS` - Main Status

Example:

```text
RAW: HSTS.
RAW: (00 B010000000000 00011000110B1172 1300.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Global System State | Unknown enum |
| 1 | Main Status Bitmask | 12-bit observed value; one observed bit appears related to utility/grid connection |
| 2 | Composite State | Multiple packed fields / sub-bitmask |
| 3 | Mode / Substatus / Code | Unknown |

Observed utility/grid connection note:

- In the captured status bitmask, one position appears to indicate utility state.
- Observed values suggest `0` = not connected and `2` = connected.
- This needs more verification before treating it as a stable field.

### `HSTS2` - Extended Status

Example:

```text
RAW: HSTS2.
RAW: (00 B1010010 11721000000 00000000000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Global State / Mode | Unknown enum |
| 1 | Main Status Bitmask | 8-bit observed value |
| 2 | Composite State | Packed fields |
| 3 | Extended Flags / Fault Bitmask | Unknown |

## EEPROM / Configuration Read Commands

### `HEEP1` - Main Configuration

Example:

```text
RAW: HEEP1.
RAW: (1 020 002 04201110230 002 1 1 0 0 0 010 020 095 050 054.3 054.3 048.0 010 000 .
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Output Source Priority Setting | Enum |
| 1 | Maximum Charging Current Setting | A |
| 2 | Maximum Mains Charging Current Setting | A |
| 3 | Packed Configuration Field | See below |
| 4 | Packed Configuration Field | See below |
| 5 | Buzzer On | Boolean |
| 6 | Backlight On | Boolean |
| 7 | Overload To Bypass Operation | Boolean / enum |
| 8 | Unknown | |
| 9 | BMS Function Enable Setting | Tentative |
| 10 | BMS Lock Machine Battery Capacity / BMS Low Power SOC | % |
| 11 | Restore Mains Charging Battery Capacity / BMS Returns To Mains Mode SOC | % |
| 12 | Restore Battery Discharging Battery Capacity / BMS Returns To Battery Mode SOC | % |
| 13 | Inverter Startup Battery Capacity / BMS Auto Start SOC After Low | % |
| 14 | Battery Constant Charging Voltage Setting | V, writable `PCVV`, observed range 48-60, typical default 56.4 |
| 15 | Battery Float Charging Voltage Setting | V, writable `PBFT`, observed range 48-60, typical default 54.0 |
| 16 | Battery Low Voltage Protection | V, writable `PSDV`, inverter Program 29 / Low DC cut-off voltage, mobile app label: Battery Cut Off Voltage Setting, observed range 40-52, step 0.1, typical default 42.0 |
| 17 | Restart Delay / Recovery Parameter | Tentative |
| 18 | Unknown | |

Observed subfields in index `3`:

| Position | Meaning | Notes |
| --- | --- | --- |
| 0 | Grid Working Range Setting | Observed `0` or `1` |
| 1 | Grid Connection Protocol Type Setting | |
| 2 | Battery Type Setting | Observed range `0` to `9` |
| 3 | Unknown | |
| 4 | Overload Automatic Restart | |
| 5 | Input Source Detection Prompt Sound | |
| 6 | Over Temperature Automatic Restart | |
| 7 | Unknown | |
| 8-10 | Rated Voltage Setting | V |

Observed subfields in index `4`:

| Position | Meaning | Notes |
| --- | --- | --- |
| 0 | Rated Frequency Setting | Hz |
| 1 | Display Automatically Returns To Homepage | |
| 2 | Charger Priority Setting | Observed range `0` to `3` |

### `HEEP2` - Battery / Schedule Configuration

Example:

```text
RAW: HEEP2.
RAW: (1 044.0 020 048.4 049.0 051.5 1 058.4 060 120 030 0000 0000 05 0000 52.0 95000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Dual Remote Switch / Parallel Mode Setting | Tentative |
| 1 | Second Shutdown Voltage | V |
| 2 | Parallel Shutdown Battery SOC / Second Output Off SOC | % |
| 3 | Low Battery Alarm Voltage / Battery Overvoltage Shutdown Voltage | V, label needs verification |
| 4 | Battery Recharge Voltage / Return To Mains Voltage | V |
| 5 | Return To Battery Voltage | V |
| 6 | Battery Equalization Mode Enable Setting | Boolean |
| 7 | Battery Equalization Voltage Setting | V |
| 8 | Battery Equalization Time Setting | minutes |
| 9 | Battery Equalization Timeout Setting | minutes |
| 10 | Battery Equalization Interval Setting | days |
| 11 | AC Charging Time | Packed schedule |
| 12 | AC Output Scheduled Time | Packed schedule |
| 13 | Restore Second Output Delay Time Setting | minutes |
| 14 | Output Open / Stop Time | Example `1122` = 11:00-22:00 |
| 15 | Restore Second Output Battery Voltage Setting | V |
| 16 | Restore Second Output Battery Capacity / Second Output Discharge Time | Tentative; observed `95000` may mean 95% in one mode |

### `HEEP3` - CT / Grid Feed Configuration

Example:

```text
RAW: HEEP3.
RAW: (2048 2048 2048 0150 053.5 03000 200 1017 000 01921 2036 0000 00000000000000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | CT Calibration / Scale Factor L1 | Tentative |
| 1 | CT Calibration / Scale Factor L2 / Backup | Tentative |
| 2 | CT Calibration / Scale Factor L3 / Average | Tentative |
| 3 | CT Zero Power | W |
| 4 | Restart Voltage After Low Power | V |
| 5 | Grid-Connected Power | W |
| 6 | Discharge Current Limit | A |
| 7 | CT Offset / Calibration Raw Value | Tentative |
| 8 | Reserved / Unknown | |
| 9 | Energy Counter / Accumulated Export | Tentative |
| 10 | Energy Counter 2 / Internal Statistic | Tentative |
| 11 | Battery Energy Grid Feeding Time | |
| 12 | Bitmask / Reserved | Unknown |

## BMS Commands

### `HBMS1` - BMS Main Values

Example:

```text
RAW: HBMS1.
RAW: (00 0000000000000000 042.8 056.8 010.0 100 0000.0 0000.0 02981 000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | BMS Status / Mode | Unknown enum |
| 1 | BMS Alarm / Warning Bitmask | Unknown |
| 2 | BMS Discharge Voltage | V |
| 3 | BMS Charge Voltage | V |
| 4 | BMS Charge Current | A |
| 5 | BMS Battery Capacity | % |
| 6 | Discharge Current Limit | Tentative; often `0` when inactive |
| 7 | Real-Time Charge Current | Tentative |
| 8 | Battery Capacity / Design Capacity | Tentative, unit unknown |
| 9 | Extended Flags / Protection Bitmask | Unknown |

### `HBMS2` - BMS Secondary Values

Example:

```text
RAW: HBMS2.
RAW: (0000.0 0000.0 0 0000 0000 0000 0000 100.0 0000000000000000.
```

| Index | Meaning | Notes |
| --- | --- | --- |
| 0 | Unknown | |
| 1 | Unknown | |
| 2 | Unknown | |
| 3 | Unknown | |
| 4 | Unknown | |
| 5 | Unknown | |
| 6 | Unknown | |
| 7 | BMS Current SOC | Tentative |
| 8 | Unknown | Flags / reserved |

### `HBMS3` - BMS Extended Values

Example:

```text
RAW: HBMS3.
RAW: (0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00000000.
```

All observed fields are currently unknown or reserved.

## Other Observed Commands

### `HPVB` - Secondary PV / PV-B Values

Example:

```text
RAW: HPVB.
RAW: (000.0 00.0 00000 0 000.0 00000000000000000000000.
```

This appears to be a secondary PV input or PV-B response. The captured values were zero, so field names are not yet verified.

### `HCTMSG1` - CT / Meter Message

Example:

```text
RAW: HCTMSG1.
RAW: (00.0 00.0 00.0 +00000 +00000 +00000 +00000 +00000 +00000 000000000000000000000.
```

All observed fields are currently unknown. The command name suggests CT or meter-related values.

## Write Command Examples

The examples below are observed write commands. They are included for research notes only.

### ECO Mode

Observed raw commands:

```text
PDjR*.
PEja..
```

Observed hex:

```text
50 44 6A 52 2A 0D
50 45 6A 61 1B 0D
```

Observed allowed values:

- Disable
- Enable

The corresponding readback field is not fully identified yet.

### BMS Function Enable Setting

Observed write commands:

```text
BMSC01X..
BMSC00H..
```

Observed readback examples from `HEEP1` suggest that enabling or disabling BMS function may change multiple values at once, including:

- A subfield inside `HEEP1` index `3`.
- `HEEP1` index `9`.
- `HEEP1` index `15` / float charging voltage.

This needs careful verification before exposing it as a user-facing switch.

### Battery Low Voltage Protection

Observed write commands for inverter Program 29 / Low DC cut-off voltage:

```text
RAW: PSDV42.23R.
RAW: PSDV43.0$ .
RAW: PSDV44.0..
```

Observed hex:

```text
50 53 44 56 34 32 2E 32 33 52 0D
50 53 44 56 34 33 2E 30 24 20 0D
50 53 44 56 34 34 2E 30 A1 B0 0D
```

Observed allowed values:

- Range `40.0` to `52.0` V.
- Step: `0.1` V.

Observed readback:

- `HEEP1` index `16`.

### Output Time Start / Stop

Observed write command:

```text
RAW: ^S???DALT1100-2200.[.
HEX: 5E 53 3F 3F 3F 44 41 4C 54 31 31 30 30 2D 32 32 30 30 9A 5B 0D
```

Observed allowed values:

- Hours `00` to `23`.
- Step: 1 hour.

Observed readback:

- `HEEP2` index `14`.
- Example `1122` appears to mean 11:00-22:00.

## Adding New Fields

When adding a newly decoded field:

1. Keep the raw captured frame for comparison.
2. Verify the value against the inverter display, Solar Plug app, or another trusted source.
3. Confirm units and scaling.
4. Add a clearly named ESPHome sensor, binary sensor, text sensor, switch, select, or button.
5. Keep unidentified fields marked as unknown instead of guessing.
6. Test that the command queue still handles timeouts and invalid responses correctly.

## Sniffer Documentation

Most of these notes were collected with a passive serial sniffer that listens to the communication between the inverter and the Solar Plug. The wiring and usage are described in [`SNIFFER.md`](SNIFFER.md), and the ESPHome configuration is in [`examples/rs232-sniffer.yaml`](../examples/rs232-sniffer.yaml). When sharing captures, include only what is useful and safe to share, and remove private or device-specific information.

## Limitations

- The protocol notes are based on tested hardware only.
- Other EASUN SMT-III firmware versions may behave differently.
- Other Solar Plug board revisions may use different wiring.
- Some fields may be decoded incorrectly until verified against more devices.
- Commands that change inverter settings should be treated with extra caution.
