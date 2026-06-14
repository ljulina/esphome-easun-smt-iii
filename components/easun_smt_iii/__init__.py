# EASUN SMT-III - ESPHome external component
#
# Monitoring and control of EASUN SMT-III (and compatible) inverters over their
# RS232 serial protocol.
#
# Project: https://github.com/ljulina/esphome-easun-smt-iii
# Author:  Ladislav Julina <ljulina@email.cz>
# License: MIT (see LICENSE)
#
# This file describes the component configuration schema and code generation.

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import button, number, select, sensor, text_sensor, time, uart
from esphome.cpp_helpers import register_parented
from esphome.const import (
    CONF_ID,
    CONF_MAX_VALUE,
    CONF_MIN_VALUE,
    CONF_MODE,
    CONF_STEP,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_HERTZ,
    UNIT_KILOWATT_HOURS,
    UNIT_MINUTE,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_VOLT_AMPS,
    UNIT_WATT,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_FREQUENCY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor", "text_sensor", "select", "number", "button"]

easun_smt_iii_ns = cg.esphome_ns.namespace("easun_smt_iii")
EasunSmtIiiComponent = easun_smt_iii_ns.class_(
    "EasunSmtIiiComponent", cg.Component, uart.UARTDevice
)
MuchgcCurrentSelect = easun_smt_iii_ns.class_(
    "MuchgcCurrentSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
OutputSourcePrioritySelect = easun_smt_iii_ns.class_(
    "OutputSourcePrioritySelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
ThreeDigitNumber = easun_smt_iii_ns.class_(
    "ThreeDigitNumber", number.Number, cg.Parented.template(EasunSmtIiiComponent)
)
FourDigitNumber = easun_smt_iii_ns.class_(
    "FourDigitNumber", number.Number, cg.Parented.template(EasunSmtIiiComponent)
)
FiveDigitNumber = easun_smt_iii_ns.class_(
    "FiveDigitNumber", number.Number, cg.Parented.template(EasunSmtIiiComponent)
)
OneDecimalNumber = easun_smt_iii_ns.class_(
    "OneDecimalNumber", number.Number, cg.Parented.template(EasunSmtIiiComponent)
)
TwoDecimalNumber = easun_smt_iii_ns.class_(
    "TwoDecimalNumber", number.Number, cg.Parented.template(EasunSmtIiiComponent)
)
AcChargingTimeStartSelect = easun_smt_iii_ns.class_(
    "AcChargingTimeStartSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
AcChargingTimeStopSelect = easun_smt_iii_ns.class_(
    "AcChargingTimeStopSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
AcOutputTimeStartSelect = easun_smt_iii_ns.class_(
    "AcOutputTimeStartSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
AcOutputTimeStopSelect = easun_smt_iii_ns.class_(
    "AcOutputTimeStopSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
OutputTimeStartSelect = easun_smt_iii_ns.class_(
    "OutputTimeStartSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
OutputTimeStopSelect = easun_smt_iii_ns.class_(
    "OutputTimeStopSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
BatteryEnergyGridFeedingTimeStartSelect = easun_smt_iii_ns.class_(
    "BatteryEnergyGridFeedingTimeStartSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
BatteryEnergyGridFeedingTimeStopSelect = easun_smt_iii_ns.class_(
    "BatteryEnergyGridFeedingTimeStopSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
GridWorkingRangeSelect = easun_smt_iii_ns.class_(
    "GridWorkingRangeSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
GridConnectionProtocolTypeSelect = easun_smt_iii_ns.class_(
    "GridConnectionProtocolTypeSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
SecondOutputRemoteActivationSelect = easun_smt_iii_ns.class_(
    "SecondOutputRemoteActivationSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
MainRemotePowerSelect = easun_smt_iii_ns.class_(
    "MainRemotePowerSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
OnOffCommandSelect = easun_smt_iii_ns.class_(
    "OnOffCommandSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
BatteryTypeSelect = easun_smt_iii_ns.class_(
    "BatteryTypeSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
EnableDisableSelect = easun_smt_iii_ns.class_(
    "EnableDisableSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
RatedFrequencySelect = easun_smt_iii_ns.class_(
    "RatedFrequencySelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
RatedVoltageSelect = easun_smt_iii_ns.class_(
    "RatedVoltageSelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
ChargerPrioritySelect = easun_smt_iii_ns.class_(
    "ChargerPrioritySelect", select.Select, cg.Parented.template(EasunSmtIiiComponent)
)
SyncInverterDatetimeButton = easun_smt_iii_ns.class_(
    "SyncInverterDatetimeButton", button.Button, cg.Parented.template(EasunSmtIiiComponent)
)
ClearFaultInformationButton = easun_smt_iii_ns.class_(
    "ClearFaultInformationButton", button.Button, cg.Parented.template(EasunSmtIiiComponent)
)
ResetFactorySettingButton = easun_smt_iii_ns.class_(
    "ResetFactorySettingButton", button.Button, cg.Parented.template(EasunSmtIiiComponent)
)
ClearPowerGenerationButton = easun_smt_iii_ns.class_(
    "ClearPowerGenerationButton", button.Button, cg.Parented.template(EasunSmtIiiComponent)
)

CONF_POLL_INTERVAL = "poll_interval"
CONF_POLL_COMMAND = "poll_command"
CONF_HGRID_POLL_INTERVAL = "hgrid_poll_interval"
CONF_HGRID_POLL_COMMAND = "hgrid_poll_command"
CONF_HBAT_POLL_INTERVAL = "hbat_poll_interval"
CONF_HBAT_POLL_COMMAND = "hbat_poll_command"
CONF_HSTS_POLL_INTERVAL = "hsts_poll_interval"
CONF_HSTS_POLL_COMMAND = "hsts_poll_command"
CONF_HPV_POLL_INTERVAL = "hpv_poll_interval"
CONF_HPV_POLL_COMMAND = "hpv_poll_command"
CONF_HTEMP_POLL_INTERVAL = "htemp_poll_interval"
CONF_HTEMP_POLL_COMMAND = "htemp_poll_command"
CONF_HEEP1_POLL_INTERVAL = "heep1_poll_interval"
CONF_HEEP1_POLL_COMMAND = "heep1_poll_command"
CONF_HBMS1_POLL_INTERVAL = "hbms1_poll_interval"
CONF_HBMS1_POLL_COMMAND = "hbms1_poll_command"
CONF_HGEN_POLL_INTERVAL = "hgen_poll_interval"
CONF_HGEN_POLL_COMMAND = "hgen_poll_command"
CONF_HPVB_POLL_INTERVAL = "hpvb_poll_interval"
CONF_HPVB_POLL_COMMAND = "hpvb_poll_command"
CONF_HEEP2_POLL_INTERVAL = "heep2_poll_interval"
CONF_HEEP2_POLL_COMMAND = "heep2_poll_command"
CONF_HIMSG1_POLL_INTERVAL = "himsg1_poll_interval"
CONF_HIMSG1_POLL_COMMAND = "himsg1_poll_command"
CONF_HBMS2_POLL_INTERVAL = "hbms2_poll_interval"
CONF_HBMS2_POLL_COMMAND = "hbms2_poll_command"
CONF_HBMS3_POLL_INTERVAL = "hbms3_poll_interval"
CONF_HBMS3_POLL_COMMAND = "hbms3_poll_command"
CONF_HEEP3_POLL_INTERVAL = "heep3_poll_interval"
CONF_HEEP3_POLL_COMMAND = "heep3_poll_command"
CONF_HSTS2_POLL_INTERVAL = "hsts2_poll_interval"
CONF_HSTS2_POLL_COMMAND = "hsts2_poll_command"
CONF_HCTMSG1_POLL_INTERVAL = "hctmsg1_poll_interval"
CONF_HCTMSG1_POLL_COMMAND = "hctmsg1_poll_command"
CONF_QPRTL_POLL_INTERVAL = "qprtl_poll_interval"
CONF_QPRTL_POLL_COMMAND = "qprtl_poll_command"
CONF_COMMAND_THROTTLE = "hgrid_tx_delay"
CONF_RESPONSE_TIMEOUT = "response_timeout"
CONF_TIME_ID = "time_id"
CONF_SYNC_INVERTER_DATETIME = "sync_inverter_datetime"
CONF_CLEAR_FAULT_INFORMATION = "clear_fault_information"
CONF_RESET_FACTORY_SETTING = "reset_factory_setting"
CONF_CLEAR_POWER_GENERATION = "clear_power_generation"

MUCHGC_OPTIONS = ["2 A", "10 A", "20 A", "30 A", "40 A", "50 A", "60 A"]

# Options for Output Source Priority Setting (POP00..POP03). Order = command value.
OUTPUT_SOURCE_PRIORITY_OPTIONS = [
    "SUB - Solar - Utility - Battery", 
    "SBU - Solar - Battery - Utility", 
    "SUF - Solar - Utility - Fast", 
    "PEC - Priority Efficiency Center"
]

# Options for Grid Working Range Select (PGR0N). Order = value N.
GRID_WORKING_RANGE_OPTIONS = ["APL - Appliances", "UPS"]

# Options for Grid Connection Protocol Type Setting (^S???RS0N). Order = N-1.
GRID_CONNECTION_PROTOCOL_TYPE_OPTIONS = ["mode1", "mode2", "mode3", "mode4", "mode5"]

# Options for Battery Type Select (PBT0N). Order = value N (0..9).
BATTERY_TYPE_OPTIONS = ["AGM", "FLD", "USE", "LIA", "PYL", "TQF", "GRO", "LIB", "LIC", "FEL"]

# Options for the binary P[DE]<c> parameters (Disable/Enable). Order = 0=Disable, 1=Enable.
ENABLE_DISABLE_OPTIONS = ["Disable", "Enable"]
WRITE_ONLY_ENABLE_DISABLE_OPTIONS = ["Write Only", "Disable", "Enable"]

# Options for Rated Frequency Select. Order = value in the composite field (0=50 Hz, 1=60 Hz).
RATED_FREQUENCY_OPTIONS = ["50 Hz", "60 Hz"]

# Options for Rated Voltage Setting (V220/V230/V240).
RATED_VOLTAGE_OPTIONS = ["220 V", "230 V", "240 V"]

# Options for Charger Priority Select (PCP0N). Order = value N (0..3).
CHARGER_PRIORITY_OPTIONS = [
    "CSO - Solar First",
    "SNU - Solar and Utility",
    "OSO - Only Solar",
    "SOR - Solar Residual",
]

HOUR_OPTIONS = [f"{hour:02d}:00" for hour in range(24)]
SECOND_OUTPUT_REMOTE_ACTIVATION_OPTIONS = ["Off", "On", "Automatic"]
MAIN_REMOTE_POWER_OPTIONS = ["Off", "On"]
ON_OFF_COMMAND_OPTIONS = ["Off", "On"]


def sensor_field(key, index, **kwargs):
    return {"key": key, "index": index, "kind": "sensor", "schema": sensor.sensor_schema(**kwargs)}


def text_field(key, index):
    return {"key": key, "index": index, "kind": "text", "schema": text_sensor.text_sensor_schema()}


def composed_text_field(key, *, kind):
    return {"key": key, "kind": kind, "schema": text_sensor.text_sensor_schema()}


def muchgc_select_field(key, index):
    return {
        "key": key,
        "index": index,
        "kind": "muchgc_select",
        "schema": select.select_schema(MuchgcCurrentSelect, icon="mdi:current-ac"),
    }


def output_source_priority_select_field(key, index):
    return {
        "key": key,
        "index": index,
        "kind": "output_source_priority_select",
        "schema": select.select_schema(OutputSourcePrioritySelect, icon="mdi:power-plug"),
    }


def dual_remote_switch_select_field(key, index):
    return {
        "key": key,
        "index": index,
        "kind": "on_off_command_select",
        "schema": select.select_schema(OnOffCommandSelect, icon="mdi:electric-switch"),
        "prefix": "PDAULC0",
        "setter": "set_dual_remote_switch_select",
    }


def hour_select_field(key, index, *, kind, select_cls, icon):
    return {
        "key": key,
        "index": index,
        "kind": kind,
        "schema": select.select_schema(select_cls, icon=icon),
    }


def on_off_command_select_field(key, index, *, prefix, setter, icon):
    return {
        "key": key,
        "index": index,
        "kind": "on_off_command_select",
        "schema": select.select_schema(OnOffCommandSelect, icon=icon),
        "prefix": prefix,
        "setter": setter,
    }


def three_digit_number_field(
    key,
    index,
    *,
    prefix,
    setter,
    unit_of_measurement,
    icon,
    min_default,
    max_default,
    step_default,
):
    """Helper for writable 3-digit parameters (MNCHGC, BMSSRC, ...).

    - Payload: "<prefix>%03u" + CRC-16/XMODEM + '\\r'
    - Readback: HEEP1 field `index` (plain number). Publishing the state is the
      responsibility of whoever registers this number (see the `set_*` setter
      referenced by `setter`).
    - `index` is also the index in HEEP1 parts[] from which the readback is read.
      Note: the current C++ code does the readback hardcoded in process_heep1_line_.
    """
    schema = number.number_schema(
        ThreeDigitNumber,
        unit_of_measurement=unit_of_measurement,
        icon=icon,
    ).extend(
        {
            cv.Optional(CONF_MIN_VALUE, default=float(min_default)): cv.float_,
            cv.Optional(CONF_MAX_VALUE, default=float(max_default)): cv.float_,
            cv.Optional(CONF_STEP, default=float(step_default)): cv.float_,
            cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        }
    )
    return {
        "key": key,
        "index": index,
        "kind": "three_digit_number",
        "schema": schema,
        "prefix": prefix,
        "setter": setter,
    }


def one_decimal_number_field(
    key,
    index,
    *,
    prefix,
    setter,
    unit_of_measurement,
    icon,
    min_default,
    max_default,
    step_default,
):
    """Helper for writable parameters in the format "<prefix>44.0" + CRC + '\\r'."""
    schema = number.number_schema(
        OneDecimalNumber,
        unit_of_measurement=unit_of_measurement,
        icon=icon,
    ).extend(
        {
            cv.Optional(CONF_MIN_VALUE, default=float(min_default)): cv.float_,
            cv.Optional(CONF_MAX_VALUE, default=float(max_default)): cv.float_,
            cv.Optional(CONF_STEP, default=float(step_default)): cv.float_,
            cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        }
    )
    return {
        "key": key,
        "index": index,
        "kind": "one_decimal_number",
        "schema": schema,
        "prefix": prefix,
        "setter": setter,
    }


def two_decimal_number_field(
    key,
    index,
    *,
    prefix,
    setter,
    unit_of_measurement,
    icon,
    min_default,
    max_default,
    step_default,
):
    """Helper for writable parameters in the format "<prefix>58.40" + CRC + '\\r'."""
    schema = number.number_schema(
        TwoDecimalNumber,
        unit_of_measurement=unit_of_measurement,
        icon=icon,
    ).extend(
        {
            cv.Optional(CONF_MIN_VALUE, default=float(min_default)): cv.float_,
            cv.Optional(CONF_MAX_VALUE, default=float(max_default)): cv.float_,
            cv.Optional(CONF_STEP, default=float(step_default)): cv.float_,
            cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        }
    )
    return {
        "key": key,
        "index": index,
        "kind": "two_decimal_number",
        "schema": schema,
        "prefix": prefix,
        "setter": setter,
    }


def four_digit_number_field(
    key,
    index,
    *,
    prefix,
    setter,
    unit_of_measurement,
    icon,
    min_default,
    max_default,
    step_default,
):
    """Helper for writable 4-digit parameters (e.g. PGFP0####)."""
    schema = number.number_schema(
        FourDigitNumber,
        unit_of_measurement=unit_of_measurement,
        icon=icon,
    ).extend(
        {
            cv.Optional(CONF_MIN_VALUE, default=float(min_default)): cv.float_,
            cv.Optional(CONF_MAX_VALUE, default=float(max_default)): cv.float_,
            cv.Optional(CONF_STEP, default=float(step_default)): cv.float_,
            cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        }
    )
    return {
        "key": key,
        "index": index,
        "kind": "four_digit_number",
        "schema": schema,
        "prefix": prefix,
        "setter": setter,
    }


def five_digit_number_field(
    key,
    index,
    *,
    prefix,
    setter,
    unit_of_measurement,
    icon,
    min_default,
    max_default,
    step_default,
):
    """Helper for writable 5-digit parameters (e.g. PGFP#####)."""
    schema = number.number_schema(
        FiveDigitNumber,
        unit_of_measurement=unit_of_measurement,
        icon=icon,
    ).extend(
        {
            cv.Optional(CONF_MIN_VALUE, default=float(min_default)): cv.float_,
            cv.Optional(CONF_MAX_VALUE, default=float(max_default)): cv.float_,
            cv.Optional(CONF_STEP, default=float(step_default)): cv.float_,
            cv.Optional(CONF_MODE, default="BOX"): cv.enum(number.NUMBER_MODES, upper=True),
        }
    )
    return {
        "key": key,
        "index": index,
        "kind": "five_digit_number",
        "schema": schema,
        "prefix": prefix,
        "setter": setter,
    }


QUERY_DEFS = [
    {
        "id": 0,
        "interval_key": CONF_POLL_INTERVAL,
        "interval_default": "10s",
        "command_key": CONF_POLL_COMMAND,
        "command_default": "HOP\r",
        "fields": [
            sensor_field(
                "output_voltage",
                0,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "output_frequency",
                1,
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_FREQUENCY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "output_apparent_power",
                2,
                unit_of_measurement=UNIT_VOLT_AMPS,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "output_active_power",
                3,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "load_percentage",
                4,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "bus_voltage",
                5,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "rated_power",
                6,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
            ),
            sensor_field(
                "inductor_current",
                7,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            text_field("status", 8),
        ],
    },
    {
        "id": 1,
        "interval_key": CONF_HGRID_POLL_INTERVAL,
        "interval_default": "10s",
        "command_key": CONF_HGRID_POLL_COMMAND,
        "command_default": "HGRID\r",
        "fields": [
            sensor_field(
                "ac_voltage",
                0,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "ac_frequency",
                1,
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_FREQUENCY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "high_point_of_mains",
                2,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
            ),
            sensor_field(
                "low_point_of_mains",
                3,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_VOLTAGE,
            ),
            sensor_field(
                "high_frequency_of_mains",
                4,
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_FREQUENCY,
            ),
            sensor_field(
                "low_frequency_of_mains",
                5,
                unit_of_measurement=UNIT_HERTZ,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_FREQUENCY,
            ),
            sensor_field("hgrid_6", 6, accuracy_decimals=0),
            sensor_field("hgrid_7", 7, accuracy_decimals=0),
            sensor_field(
                "hgrid_rated_power",
                8,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
            ),
            text_field("hgrid_9", 9),
        ],
    },
    {
        "id": 2,
        "interval_key": CONF_HBAT_POLL_INTERVAL,
        "interval_default": "10s",
        "command_key": CONF_HBAT_POLL_COMMAND,
        "command_default": "HBAT\r",
        "fields": [
            sensor_field("battery_number_in_series", 0, accuracy_decimals=0),
            sensor_field(
                "battery_voltage",
                1,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "battery_capacity",
                2,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "battery_charging_current",
                3,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "battery_discharging_current",
                4,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "hbat_bus_voltage",
                5,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            text_field("hbat_6", 6),
            text_field("hbat_7", 7),
        ],
    },
    {
        "id": 3,
        "interval_key": CONF_HSTS_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HSTS_POLL_COMMAND,
        "command_default": "HSTS\r",
        "fields": [text_field(f"hsts_{i}", i) for i in range(4)],
    },
    {
        "id": 4,
        "interval_key": CONF_HPV_POLL_INTERVAL,
        "interval_default": "10s",
        "command_key": CONF_HPV_POLL_COMMAND,
        "command_default": "HPV\r",
        "fields": [
            sensor_field(
                "pv_voltage",
                0,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "pv_current",
                1,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "pv_power",
                2,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            text_field("hpv_3", 3),
            text_field("hpv_4", 4),
            text_field("hpv_5", 5),
            sensor_field(
                "pv_max_voltage",
                6,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
            ),
            sensor_field(
                "pv_temperature",
                7,
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "pv_max_power",
                8,
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        ],
    },
    {
        "id": 5,
        "interval_key": CONF_HTEMP_POLL_INTERVAL,
        "interval_default": "10s",
        "command_key": CONF_HTEMP_POLL_COMMAND,
        "command_default": "HTEMP\r",
        "fields": [
            # HTEMP response: fields 0..4 are temperatures in degC, fields 5..6
            # are fan speeds in %. Field 7 is kept as raw text.
            *[
                sensor_field(
                    key,
                    idx,
                    unit_of_measurement=UNIT_CELSIUS,
                    accuracy_decimals=0,
                    device_class=DEVICE_CLASS_TEMPERATURE,
                    state_class=STATE_CLASS_MEASUREMENT,
                )
                for idx, key in enumerate(
                    [
                        "pv_temperature_2",
                        "inverter_temperature",
                        "boost_temperature",
                        "htemp_3",
                        "htemp_4",
                    ]
                )
            ],
            sensor_field(
                "fan_1_speed",
                5,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "fan_2_speed",
                6,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            text_field("htemp_7", 7),
        ],
    },
    {
        "id": 6,
        "interval_key": CONF_HEEP1_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HEEP1_POLL_COMMAND,
        "command_default": "HEEP1\r",
        "fields": [
            output_source_priority_select_field("output_source_priority_setting", 0),
            three_digit_number_field(
                "max_charging_current",
                1,
                prefix="MNCHGC",
                setter="set_max_charging_current_number",
                unit_of_measurement=UNIT_AMPERE,
                icon="mdi:current-dc",
                min_default=10,
                max_default=120,
                step_default=10,
            ),
            muchgc_select_field("maximum_mains_charging_current_settings", 2),
            # HEEP1 fields 3 and 4 are composite strings; they are split into
            # several TOP_LEVEL_ENTITIES (grid_working_range, battery_type,
            # overload/over_temp auto restart, input_source_detection_prompt_sound,
            # rated_voltage, rated_frequency, display_auto_return_homepage, charger_priority).
            # They are not exposed here as sensors.
            # HEEP1 field 5 (Buzzer On) and field 6 (Backlight On) are likewise
            # exposed as top-level enable_disable_select, not as text sensors.
            text_field("heep1_7", 7),
            text_field("heep1_8", 8),
            on_off_command_select_field(
                "bms_function_enable_setting",
                9,
                prefix="BMSC0",
                setter="set_bms_function_enable_setting_select",
                icon="mdi:battery-check",
            ),
            three_digit_number_field(
                "bms_lock_machine_battery_capacity",
                10,
                prefix="BMSSDC",
                setter="set_bms_lock_machine_battery_capacity_number",
                unit_of_measurement=UNIT_PERCENT,
                icon="mdi:battery-lock",
                min_default=5,
                max_default=90,
                step_default=5,
            ),
            three_digit_number_field(
                "restore_mains_charging_battery_capacity",
                11,
                prefix="BMSB2UC",
                setter="set_restore_mains_charging_battery_capacity_number",
                unit_of_measurement=UNIT_PERCENT,
                icon="mdi:battery-arrow-up",
                min_default=25,
                max_default=95,
                step_default=5,
            ),
            three_digit_number_field(
                "restore_battery_discharging_battery_capacity",
                12,
                prefix="BMSU2BC",
                setter="set_restore_battery_discharging_battery_capacity_number",
                unit_of_measurement=UNIT_PERCENT,
                icon="mdi:battery-arrow-down",
                min_default=30,
                max_default=95,
                step_default=5,
            ),
            three_digit_number_field(
                "inverter_startup_battery_capacity",
                13,
                prefix="BMSSRC",
                setter="set_inverter_startup_battery_capacity_number",
                unit_of_measurement=UNIT_PERCENT,
                icon="mdi:battery-heart",
                min_default=5,
                max_default=100,
                step_default=5,
            ),
            # HEEP1[14] PCVV / HEEP1[15] PBFT: observed inverter UI range 48-60 V;
            # typical factory defaults 56.4 V and 54.0 V (verify on your hardware).
            one_decimal_number_field(
                "battery_constant_charging_voltage_setting",
                14,
                prefix="PCVV",
                setter="set_battery_constant_charging_voltage_setting_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-charging-high",
                min_default=48.0,
                max_default=60.0,
                step_default=0.1,
            ),
            one_decimal_number_field(
                "battery_float_charging_voltage_setting",
                15,
                prefix="PBFT",
                setter="set_battery_float_charging_voltage_setting_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-charging",
                min_default=48.0,
                max_default=60.0,
                step_default=0.1,
            ),
            one_decimal_number_field(
                "battery_low_voltage_protection",
                16,
                prefix="PSDV",
                setter="set_battery_low_voltage_protection_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-alert-variant-outline",
                min_default=40.0,
                max_default=52.0,
                step_default=0.1,
            ),
            text_field("heep1_17", 17),
            text_field("heep1_18", 18),
        ],
    },
    {
        "id": 7,
        "interval_key": CONF_HBMS1_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HBMS1_POLL_COMMAND,
        "command_default": "HBMS1\r",
        "fields": [
            sensor_field("hbms1_0", 0, accuracy_decimals=0),
            text_field("hbms1_1", 1),
            sensor_field(
                "bms_discharge_voltage",
                2,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "bms_charge_voltage",
                3,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "bms_charge_current",
                4,
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field(
                "bms_battery_capacity",
                5,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            sensor_field("hbms1_6", 6, accuracy_decimals=1),
            sensor_field("hbms1_7", 7, accuracy_decimals=1),
            sensor_field("hbms1_8", 8, accuracy_decimals=0),
            sensor_field("hbms1_9", 9, accuracy_decimals=0),
        ],
    },
    {
        "id": 8,
        "interval_key": CONF_HGEN_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HGEN_POLL_COMMAND,
        "command_default": "HGEN\r",
        "fields": [
            composed_text_field("system_time", kind="system_time_text"),
            # PV energy readings. `total_power_generation_total_pv` is safe to
            # mark as TOTAL_INCREASING (HA Energy Dashboard). The daily/monthly/
            # yearly counters reset over time, so we leave their state_class unset
            # and the user can add one if needed.
            sensor_field(
                "daily_power_generation_total_pv",
                2,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_ENERGY,
            ),
            sensor_field(
                "month_power_generation_total_pv",
                3,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ENERGY,
            ),
            sensor_field(
                "year_power_generation_total_pv",
                4,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ENERGY,
            ),
            sensor_field(
                "total_power_generation_total_pv",
                5,
                unit_of_measurement=UNIT_KILOWATT_HOURS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_TOTAL_INCREASING,
            ),
            text_field("hgen_6", 6),
        ],
    },
    {
        "id": 9,
        "interval_key": CONF_HPVB_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HPVB_POLL_COMMAND,
        "command_default": "HPVB\r",
        "fields": [text_field(f"hpvb_{i}", i) for i in range(6)],
    },
    {
        "id": 10,
        "interval_key": CONF_HEEP2_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HEEP2_POLL_COMMAND,
        "command_default": "HEEP2\r",
        "fields": [
            dual_remote_switch_select_field("dual_remote_switch", 0),
            sensor_field(
                "second_shutdown_voltage",
                1,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
            ),
            sensor_field(
                "parallel_shutdown_battery_soc_settings",
                2,
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
            ),
            one_decimal_number_field(
                "low_battery_alarm_voltage_setting",
                3,
                prefix="PSLV",
                setter="set_low_battery_alarm_voltage_setting_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-alert",
                min_default=40,
                max_default=54,
                step_default=1,
            ),
            # Keep the historical YAML keys even though the HA/app labels use
            # "Battery Recharge/Redischarge Voltage Setting".
            one_decimal_number_field(
                "return_to_mains_voltage",
                4,
                prefix="PBCV",
                setter="set_battery_recharge_voltage_setting_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-plus",
                min_default=44,
                max_default=51,
                step_default=1,
            ),
            one_decimal_number_field(
                "return_to_battery_voltage",
                5,
                prefix="PBDV",
                setter="set_battery_redischarge_voltage_setting_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-minus",
                min_default=48,
                max_default=58,
                step_default=1,
            ),
            on_off_command_select_field(
                "battery_equalization_mode_enable_setting",
                6,
                prefix="PBEQE",
                setter="set_battery_equalization_mode_enable_setting_select",
                icon="mdi:battery-sync",
            ),
            two_decimal_number_field(
                "battery_equalization_voltage_settings",
                7,
                prefix="PBEQV",
                setter="set_battery_equalization_voltage_setting_number",
                unit_of_measurement=UNIT_VOLT,
                icon="mdi:battery-sync",
                min_default=48,
                max_default=60,
                step_default=0.1,
            ),
            three_digit_number_field(
                "battery_equalization_time_settings",
                8,
                prefix="PBEQT",
                setter="set_battery_equalization_time_number",
                unit_of_measurement=UNIT_MINUTE,
                icon="mdi:timer-outline",
                min_default=5,
                max_default=900,
                step_default=5,
            ),
            three_digit_number_field(
                "battery_equalization_timeout_settings",
                9,
                prefix="PBEQOT",
                setter="set_battery_equalization_timeout_number",
                unit_of_measurement=UNIT_MINUTE,
                icon="mdi:timer-alert-outline",
                min_default=5,
                max_default=900,
                step_default=5,
            ),
            three_digit_number_field(
                "battery_equalization_interval_settings",
                10,
                prefix="PBEQP",
                setter="set_battery_equalization_interval_number",
                unit_of_measurement="d",
                icon="mdi:calendar-sync",
                min_default=0,
                max_default=90,
                step_default=1,
            ),
            hour_select_field(
                "ac_charging_time_start",
                11,
                kind="ac_charging_time_start_select",
                select_cls=AcChargingTimeStartSelect,
                icon="mdi:clock-start",
            ),
            hour_select_field(
                "ac_charging_time_stop",
                11,
                kind="ac_charging_time_stop_select",
                select_cls=AcChargingTimeStopSelect,
                icon="mdi:clock-end",
            ),
            hour_select_field(
                "ac_output_time_start",
                12,
                kind="ac_output_time_start_select",
                select_cls=AcOutputTimeStartSelect,
                icon="mdi:clock-start",
            ),
            hour_select_field(
                "ac_output_time_stop",
                12,
                kind="ac_output_time_stop_select",
                select_cls=AcOutputTimeStopSelect,
                icon="mdi:clock-end",
            ),
            sensor_field(
                "restore_second_output_delay_time_settings",
                13,
                unit_of_measurement=UNIT_MINUTE,
                accuracy_decimals=0,
            ),
            hour_select_field(
                "output_time_start",
                14,
                kind="output_time_start_select",
                select_cls=OutputTimeStartSelect,
                icon="mdi:clock-start",
            ),
            hour_select_field(
                "output_time_stop",
                14,
                kind="output_time_stop_select",
                select_cls=OutputTimeStopSelect,
                icon="mdi:clock-end",
            ),
            sensor_field(
                "restore_second_output_battery_voltage_settings",
                15,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
            ),
            # HEEP2 field 16 is a single 5-digit composite (e.g. "95010"): the
            # first 2 digits are the battery capacity in %, the last 3 digits are
            # the discharge time in minutes. It is split into the two number
            # entities below. The `index=16` value is the same for both on purpose
            # but is not actually used for number kinds (readback is hardcoded in
            # process_heep2_line_; registration happens via the setter).
            three_digit_number_field(
                "restore_second_output_battery_capacity_settings",
                16,
                prefix="PDSRS",
                setter="set_restore_second_output_battery_capacity_number",
                unit_of_measurement=UNIT_PERCENT,
                icon="mdi:battery-sync",
                min_default=25,
                max_default=95,
                step_default=5,
            ),
            four_digit_number_field(
                "second_output_discharge_time_settings",
                16,
                prefix="PDDCGT",
                setter="set_second_output_discharge_time_number",
                unit_of_measurement=UNIT_MINUTE,
                icon="mdi:timer-outline",
                min_default=0,
                max_default=990,
                step_default=5,
            ),
        ],
    },
    {
        "id": 11,
        "interval_key": CONF_HIMSG1_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HIMSG1_POLL_COMMAND,
        "command_default": "HIMSG1\r",
        "fields": [
            # 0: Software Version (e.g. "0020.04")
            text_field("software_version", 0),
            # 1: Software Date (YYYYMMDD, e.g. "20260108")
            text_field("software_date", 1),
            # 2: not yet identified (on the test firmware it returns "12" - maybe
            # the HW/protocol version or a model code). Kept as a text_sensor.
            text_field("himsg1_2", 2),
        ],
    },
    {
        "id": 12,
        "interval_key": CONF_HBMS2_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HBMS2_POLL_COMMAND,
        "command_default": "HBMS2\r",
        "fields": [text_field(f"hbms2_{i}", i) for i in range(9)],
    },
    {
        "id": 13,
        "interval_key": CONF_HBMS3_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HBMS3_POLL_COMMAND,
        "command_default": "HBMS3\r",
        "fields": [text_field(f"hbms3_{i}", i) for i in range(17)],
    },
    {
        "id": 14,
        "interval_key": CONF_HEEP3_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HEEP3_POLL_COMMAND,
        "command_default": "HEEP3\r",
        "fields": [
            text_field("heep3_0", 0),
            text_field("heep3_1", 1),
            text_field("heep3_2", 2),
            three_digit_number_field(
                "ct_zero_power",
                3,
                prefix="EZCTP0",
                setter="set_ct_zero_power_number",
                unit_of_measurement=UNIT_WATT,
                icon="mdi:flash-outline",
                min_default=10,
                max_default=500,
                step_default=10,
            ),
            sensor_field(
                "restart_voltage_after_low_power",
                4,
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
            ),
            five_digit_number_field(
                "grid_connected_power",
                5,
                prefix="PGFP",
                setter="set_grid_connected_power_number",
                unit_of_measurement=UNIT_WATT,
                icon="mdi:transmission-tower-import",
                min_default=200,
                max_default=6200,
                step_default=100,
            ),
            three_digit_number_field(
                "discharge_current_limit",
                6,
                prefix="DISCC",
                setter="set_discharge_current_limit_number",
                unit_of_measurement=UNIT_AMPERE,
                icon="mdi:current-dc",
                min_default=20,
                max_default=200,
                step_default=1,
            ),
            text_field("heep3_7", 7),
            text_field("heep3_8", 8),
            text_field("heep3_9", 9),
            text_field("heep3_10", 10),
            hour_select_field(
                "battery_energy_grid_feeding_time_start",
                11,
                kind="battery_energy_grid_feeding_time_start_select",
                select_cls=BatteryEnergyGridFeedingTimeStartSelect,
                icon="mdi:home-battery-outline",
            ),
            hour_select_field(
                "battery_energy_grid_feeding_time_stop",
                11,
                kind="battery_energy_grid_feeding_time_stop_select",
                select_cls=BatteryEnergyGridFeedingTimeStopSelect,
                icon="mdi:home-battery-outline",
            ),
            text_field("heep3_12", 12),
        ],
    },
    {
        "id": 15,
        "interval_key": CONF_HSTS2_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HSTS2_POLL_COMMAND,
        "command_default": "HSTS2\r",
        "fields": [text_field(f"hsts2_{i}", i) for i in range(4)],
    },
    {
        "id": 16,
        "interval_key": CONF_HCTMSG1_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_HCTMSG1_POLL_COMMAND,
        "command_default": "HCTMSG1\r",
        "fields": [text_field(f"hctmsg1_{i}", i) for i in range(10)],
    },
    {
        "id": 17,
        "interval_key": CONF_QPRTL_POLL_INTERVAL,
        "interval_default": "60s",
        "command_key": CONF_QPRTL_POLL_COMMAND,
        "command_default": "QPRTL\r",
        "fields": [text_field("device_type", 0)],
    },
]

# Top-level entities - these are not bound to a single index of a poll response;
# their readback is done by special parsing (e.g. HEEP1 field 3 is a composite
# string where each position represents a different setting).
TOP_LEVEL_ENTITIES = [
    {
        "key": "grid_working_range",
        "kind": "grid_working_range_select",
        "schema": select.select_schema(GridWorkingRangeSelect, icon="mdi:transmission-tower"),
    },
    {
        "key": "grid_connection_protocol_type_setting",
        "kind": "grid_connection_protocol_type_select",
        "schema": select.select_schema(
            GridConnectionProtocolTypeSelect,
            icon="mdi:transmission-tower-export",
        ),
    },
    {
        "key": "battery_type",
        "kind": "battery_type_select",
        "schema": select.select_schema(BatteryTypeSelect, icon="mdi:car-battery"),
    },
    {
        "key": "overload_auto_restart",
        "kind": "enable_disable_select",
        "param_char": "u",
        "setter": "set_overload_auto_restart_select",
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:restart-alert"),
    },
    {
        "key": "over_temperature_auto_restart",
        "kind": "enable_disable_select",
        "param_char": "v",
        "setter": "set_over_temp_auto_restart_select",
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:thermometer-alert"),
    },
    {
        "key": "buzzer_on",
        "kind": "enable_disable_select",
        "param_char": "a",
        "setter": "set_buzzer_on_select",
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:bell-ring"),
    },
    {
        "key": "backlight_on",
        "kind": "enable_disable_select",
        "param_char": "x",
        "setter": "set_backlight_on_select",
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:brightness-6"),
    },
    {
        "key": "display_auto_return_homepage",
        "kind": "enable_disable_select",
        "param_char": "k",
        "setter": "set_display_auto_return_homepage_select",
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:monitor-dashboard"),
    },
    {
        "key": "input_source_detection_prompt_sound",
        "kind": "enable_disable_select",
        "param_char": "y",
        "setter": "set_input_source_detection_prompt_sound_select",
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:volume-medium"),
    },
    # ECO Mode (PDj/PEj) - we don't yet know where the current state is read in HEEP1.
    {
        "key": "eco_mode",
        "kind": "enable_disable_select",
        "param_char": "j",
        "setter": "set_eco_mode_select",
        "write_only_option": True,
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:leaf"),
    },
    # Fault Information Storage (P[DE]z) - we don't yet know how to read the
    # current state. The entity writes (optimistic publish), but after boot it
    # doesn't know the inverter state. Once a position in some HEEP* is found,
    # we can add the readback.
    {
        "key": "fault_information_storage",
        "kind": "enable_disable_select",
        "param_char": "z",
        "setter": "set_fault_information_storage_select",
        "write_only_option": True,
        "schema": select.select_schema(EnableDisableSelect, icon="mdi:database-outline"),
    },
    # Remote Activation Of The Second Output (DAULREMOTESW0/1/2) - readback field
    # not yet known. Behaves like fault_information_storage: optimistic write-only.
    {
        "key": "remote_activation_of_the_second_output",
        "kind": "second_output_remote_activation_select",
        "schema": select.select_schema(
            SecondOutputRemoteActivationSelect,
            icon="mdi:power-plug-outline",
        ),
    },
    # Remote Power On (MAINREMOTESW0/1) - readback field not yet known.
    {
        "key": "remote_power_on",
        "kind": "main_remote_power_select",
        "schema": select.select_schema(MainRemotePowerSelect, icon="mdi:power"),
    },
    {
        "key": "rated_frequency",
        "kind": "rated_frequency_select",
        "schema": select.select_schema(RatedFrequencySelect, icon="mdi:sine-wave"),
    },
    {
        "key": "rated_voltage",
        "kind": "rated_voltage_select",
        "schema": select.select_schema(RatedVoltageSelect, icon="mdi:transmission-tower"),
    },
    {
        "key": "charger_priority",
        "kind": "charger_priority_select",
        "schema": select.select_schema(ChargerPrioritySelect, icon="mdi:solar-power"),
    },
]


CONFIG_DICT = {
    cv.GenerateID(): cv.declare_id(EasunSmtIiiComponent),
    cv.Optional(CONF_COMMAND_THROTTLE, default="200ms"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_RESPONSE_TIMEOUT, default="3s"): cv.positive_time_period_milliseconds,
    # Reference to the time: component. Required for sync_inverter_datetime
    # (we read the current time from it and send it to the inverter).
    cv.Optional(CONF_TIME_ID): cv.use_id(time.RealTimeClock),
    # "Sync Inverter Datetime From HA" button - when pressed it sends the
    # ^S???DAT<YYMMDDhhmmss> command with the current time from the time: component.
    cv.Optional(CONF_SYNC_INVERTER_DATETIME): button.button_schema(
        SyncInverterDatetimeButton, icon="mdi:clock-check-outline"
    ),
    # "Clear Fault Information" button - when pressed it sends the FAULTCG command.
    cv.Optional(CONF_CLEAR_FAULT_INFORMATION): button.button_schema(
        ClearFaultInformationButton, icon="mdi:alert-remove-outline"
    ),
    # "Reset Factory Setting" button - when pressed it sends the PF command.
    cv.Optional(CONF_RESET_FACTORY_SETTING): button.button_schema(
        ResetFactorySettingButton, icon="mdi:factory"
    ),
    # "Clear Power Generation" button - when pressed it sends the ^S???CLE command.
    cv.Optional(CONF_CLEAR_POWER_GENERATION): button.button_schema(
        ClearPowerGenerationButton, icon="mdi:solar-power-variant-outline"
    ),
}

for query in QUERY_DEFS:
    CONFIG_DICT[cv.Optional(query["interval_key"], default=query["interval_default"])] = (
        cv.positive_time_period_milliseconds
    )
    CONFIG_DICT[cv.Optional(query["command_key"], default=query["command_default"])] = cv.string
    for field in query["fields"]:
        CONFIG_DICT[cv.Optional(field["key"])] = field["schema"]

for entity in TOP_LEVEL_ENTITIES:
    CONFIG_DICT[cv.Optional(entity["key"])] = entity["schema"]


CONFIG_SCHEMA = (
    cv.Schema(CONFIG_DICT).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_command_throttle_ms(config[CONF_COMMAND_THROTTLE]))
    cg.add(var.set_response_timeout_ms(config[CONF_RESPONSE_TIMEOUT]))

    if CONF_TIME_ID in config:
        time_var = await cg.get_variable(config[CONF_TIME_ID])
        cg.add(var.set_time_source(time_var))

    if CONF_SYNC_INVERTER_DATETIME in config:
        btn = await button.new_button(config[CONF_SYNC_INVERTER_DATETIME])
        await register_parented(btn, var)

    if CONF_CLEAR_FAULT_INFORMATION in config:
        btn = await button.new_button(config[CONF_CLEAR_FAULT_INFORMATION])
        await register_parented(btn, var)

    if CONF_RESET_FACTORY_SETTING in config:
        btn = await button.new_button(config[CONF_RESET_FACTORY_SETTING])
        await register_parented(btn, var)

    if CONF_CLEAR_POWER_GENERATION in config:
        btn = await button.new_button(config[CONF_CLEAR_POWER_GENERATION])
        await register_parented(btn, var)

    for query in QUERY_DEFS:
        query_id = query["id"]
        cg.add(var.set_query_interval_ms(query_id, config[query["interval_key"]]))
        cg.add(var.set_query_command(query_id, config[query["command_key"]]))

        enabled = False
        for field in query["fields"]:
            if field["key"] in config:
                enabled = True
                if field["kind"] == "sensor":
                    s = await sensor.new_sensor(config[field["key"]])
                    cg.add(var.set_numeric_sensor(query_id, field["index"], s))
                elif field["kind"] == "muchgc_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=MUCHGC_OPTIONS)
                    cg.add(var.set_maximum_mains_charging_current_select(sel))
                elif field["kind"] == "output_source_priority_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=OUTPUT_SOURCE_PRIORITY_OPTIONS)
                    cg.add(var.set_output_source_priority_select(sel))
                elif field["kind"] == "ac_charging_time_start_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_ac_charging_time_start_select(sel))
                elif field["kind"] == "ac_charging_time_stop_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_ac_charging_time_stop_select(sel))
                elif field["kind"] == "ac_output_time_start_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_ac_output_time_start_select(sel))
                elif field["kind"] == "ac_output_time_stop_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_ac_output_time_stop_select(sel))
                elif field["kind"] == "output_time_start_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_output_time_start_select(sel))
                elif field["kind"] == "output_time_stop_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_output_time_stop_select(sel))
                elif field["kind"] == "battery_energy_grid_feeding_time_start_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_battery_energy_grid_feeding_time_start_select(sel))
                elif field["kind"] == "battery_energy_grid_feeding_time_stop_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=HOUR_OPTIONS)
                    cg.add(var.set_battery_energy_grid_feeding_time_stop_select(sel))
                elif field["kind"] == "on_off_command_select":
                    sel = cg.new_Pvariable(config[field["key"]][CONF_ID])
                    await register_parented(sel, var)
                    await select.register_select(sel, config[field["key"]], options=ON_OFF_COMMAND_OPTIONS)
                    cg.add(sel.set_command_prefix(field["prefix"]))
                    cg.add(getattr(var, field["setter"])(sel))
                elif field["kind"] == "system_time_text":
                    t = await text_sensor.new_text_sensor(config[field["key"]])
                    cg.add(var.set_system_time_text_sensor(t))
                elif field["kind"] == "three_digit_number":
                    conf = config[field["key"]]
                    num = await number.new_number(
                        conf,
                        min_value=conf[CONF_MIN_VALUE],
                        max_value=conf[CONF_MAX_VALUE],
                        step=conf[CONF_STEP],
                    )
                    await register_parented(num, var)
                    cg.add(num.set_command_prefix(field["prefix"]))
                    cg.add(getattr(var, field["setter"])(num))
                elif field["kind"] == "one_decimal_number":
                    conf = config[field["key"]]
                    num = await number.new_number(
                        conf,
                        min_value=conf[CONF_MIN_VALUE],
                        max_value=conf[CONF_MAX_VALUE],
                        step=conf[CONF_STEP],
                    )
                    await register_parented(num, var)
                    cg.add(num.set_command_prefix(field["prefix"]))
                    cg.add(getattr(var, field["setter"])(num))
                elif field["kind"] == "two_decimal_number":
                    conf = config[field["key"]]
                    num = await number.new_number(
                        conf,
                        min_value=conf[CONF_MIN_VALUE],
                        max_value=conf[CONF_MAX_VALUE],
                        step=conf[CONF_STEP],
                    )
                    await register_parented(num, var)
                    cg.add(num.set_command_prefix(field["prefix"]))
                    cg.add(getattr(var, field["setter"])(num))
                elif field["kind"] == "four_digit_number":
                    conf = config[field["key"]]
                    num = await number.new_number(
                        conf,
                        min_value=conf[CONF_MIN_VALUE],
                        max_value=conf[CONF_MAX_VALUE],
                        step=conf[CONF_STEP],
                    )
                    await register_parented(num, var)
                    cg.add(num.set_command_prefix(field["prefix"]))
                    cg.add(getattr(var, field["setter"])(num))
                elif field["kind"] == "five_digit_number":
                    conf = config[field["key"]]
                    num = await number.new_number(
                        conf,
                        min_value=conf[CONF_MIN_VALUE],
                        max_value=conf[CONF_MAX_VALUE],
                        step=conf[CONF_STEP],
                    )
                    await register_parented(num, var)
                    cg.add(num.set_command_prefix(field["prefix"]))
                    cg.add(getattr(var, field["setter"])(num))
                else:
                    t = await text_sensor.new_text_sensor(config[field["key"]])
                    cg.add(var.set_text_sensor(query_id, field["index"], t))

        cg.add(var.set_query_enabled(query_id, enabled))

    for entity in TOP_LEVEL_ENTITIES:
        if entity["key"] not in config:
            continue
        conf = config[entity["key"]]
        if entity["kind"] == "grid_working_range_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=GRID_WORKING_RANGE_OPTIONS)
            cg.add(var.set_grid_working_range_select(sel))
        elif entity["kind"] == "grid_connection_protocol_type_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=GRID_CONNECTION_PROTOCOL_TYPE_OPTIONS)
            cg.add(var.set_grid_connection_protocol_type_select(sel))
        elif entity["kind"] == "battery_type_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=BATTERY_TYPE_OPTIONS)
            cg.add(var.set_battery_type_select(sel))
        elif entity["kind"] == "second_output_remote_activation_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=SECOND_OUTPUT_REMOTE_ACTIVATION_OPTIONS)
            cg.add(var.set_second_output_remote_activation_select(sel))
        elif entity["kind"] == "main_remote_power_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=MAIN_REMOTE_POWER_OPTIONS)
            cg.add(var.set_main_remote_power_select(sel))
        elif entity["kind"] == "enable_disable_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            write_only_option = entity.get("write_only_option", False)
            options = WRITE_ONLY_ENABLE_DISABLE_OPTIONS if write_only_option else ENABLE_DISABLE_OPTIONS
            await select.register_select(sel, conf, options=options)
            cg.add(sel.set_param_char(ord(entity["param_char"])))
            cg.add(sel.set_write_only_option(write_only_option))
            cg.add(getattr(var, entity["setter"])(sel))
        elif entity["kind"] == "rated_frequency_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=RATED_FREQUENCY_OPTIONS)
            cg.add(var.set_rated_frequency_select(sel))
        elif entity["kind"] == "rated_voltage_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=RATED_VOLTAGE_OPTIONS)
            cg.add(var.set_rated_voltage_select(sel))
        elif entity["kind"] == "charger_priority_select":
            sel = cg.new_Pvariable(conf[CONF_ID])
            await register_parented(sel, var)
            await select.register_select(sel, conf, options=CHARGER_PRIORITY_OPTIONS)
            cg.add(var.set_charger_priority_select(sel))
