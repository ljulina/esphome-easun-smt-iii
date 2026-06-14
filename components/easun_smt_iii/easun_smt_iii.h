#pragma once

// EASUN SMT-III - ESPHome external component
//
// Monitoring and control of EASUN SMT-III (and compatible) inverters over their
// RS232 serial protocol.
//
// Project: https://github.com/ljulina/esphome-easun-smt-iii
// Author:  Ladislav Julina <ljulina@email.cz>
// License: MIT (see LICENSE)

#include <array>
#include <deque>
#include <string>
#include <vector>

#include "esphome/core/component.h"
#include "esphome/core/time.h"
#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace easun_smt_iii {

class MuchgcCurrentSelect;
class OutputSourcePrioritySelect;
class ThreeDigitNumber;
class FourDigitNumber;
class FiveDigitNumber;
class OneDecimalNumber;
class TwoDecimalNumber;
class AcChargingTimeStartSelect;
class AcChargingTimeStopSelect;
class AcOutputTimeStartSelect;
class AcOutputTimeStopSelect;
class OutputTimeStartSelect;
class OutputTimeStopSelect;
class BatteryEnergyGridFeedingTimeStartSelect;
class BatteryEnergyGridFeedingTimeStopSelect;
class GridWorkingRangeSelect;
class GridConnectionProtocolTypeSelect;
class SecondOutputRemoteActivationSelect;
class MainRemotePowerSelect;
class OnOffCommandSelect;
class BatteryTypeSelect;
class EnableDisableSelect;
class RatedFrequencySelect;
class RatedVoltageSelect;
class ChargerPrioritySelect;
class SyncInverterDatetimeButton;
class ClearFaultInformationButton;
class ResetFactorySettingButton;
class ClearPowerGenerationButton;

enum class ExpectedResponse : uint8_t {
  HOP = 0,
  HGRID = 1,
  HBAT = 2,
  HSTS = 3,
  HPV = 4,
  HTEMP = 5,
  HEEP1 = 6,
  HBMS1 = 7,
  HGEN = 8,
  HPVB = 9,
  HEEP2 = 10,
  HIMSG1 = 11,
  HBMS2 = 12,
  HBMS3 = 13,
  HEEP3 = 14,
  HSTS2 = 15,
  HCTMSG1 = 16,
  QPRTL = 17,
};

class EasunSmtIiiComponent : public uart::UARTDevice, public Component {
 public:
  EasunSmtIiiComponent();

  void setup() override;
  void loop() override;
  void dump_config() override;

  void set_query_enabled(uint8_t kind, bool enabled);
  void set_query_interval_ms(uint8_t kind, uint32_t ms);
  void set_query_command(uint8_t kind, const std::string &cmd);
  void set_numeric_sensor(uint8_t kind, uint8_t index, sensor::Sensor *s);
  void set_text_sensor(uint8_t kind, uint8_t index, text_sensor::TextSensor *s);
  void set_system_time_text_sensor(text_sensor::TextSensor *t) { this->system_time_text_sensor_ = t; }
  void queue_muchgc_current_setting(uint16_t amps);
  void queue_output_source_priority_setting(uint8_t code);
  // Generic writable 3-digit parameter: "<prefix>%03u" + CRC + '\r'.
  // Used for MNCHGC, BMSSRC, ... min/max/step validation already happens in ThreeDigitNumber.
  void queue_three_digit_setting(const char *prefix, uint16_t value);
  // Generic writable 4-digit parameter: "<prefix>%04u" + CRC + '\r'.
  void queue_four_digit_setting(const char *prefix, uint16_t value);
  // Generic writable 5-digit parameter: "<prefix>%05u" + CRC + '\r'.
  void queue_five_digit_setting(const char *prefix, uint16_t value);
  // Generic writable parameter with one decimal place: "<prefix>44.0" + CRC + '\r'.
  void queue_one_decimal_setting(const char *prefix, uint16_t value_tenths);
  // Generic writable parameter with two decimal places: "<prefix>58.40" + CRC + '\r'.
  void queue_two_decimal_setting(const char *prefix, uint16_t value_hundredths);
  void queue_ac_charging_time_setting(uint8_t start_hour, uint8_t end_hour);
  void queue_ac_output_time_setting(uint8_t start_hour, uint8_t end_hour);
  void queue_output_time_setting(uint8_t start_hour, uint8_t end_hour);
  void queue_battery_energy_grid_feeding_time_setting(uint8_t start_hour, uint8_t end_hour);
  void queue_ac_charging_time_start_setting(uint8_t start_hour);
  void queue_ac_charging_time_stop_setting(uint8_t stop_hour);
  void queue_ac_output_time_start_setting(uint8_t start_hour);
  void queue_ac_output_time_stop_setting(uint8_t stop_hour);
  void queue_output_time_start_setting(uint8_t start_hour);
  void queue_output_time_stop_setting(uint8_t stop_hour);
  void queue_battery_energy_grid_feeding_time_start_setting(uint8_t start_hour);
  void queue_battery_energy_grid_feeding_time_stop_setting(uint8_t stop_hour);
  void queue_grid_working_range_setting(uint8_t code);
  void queue_grid_connection_protocol_type_setting(uint8_t mode);
  void queue_second_output_remote_activation_setting(uint8_t mode);
  void queue_main_remote_power_setting(bool on);
  void queue_on_off_command_setting(const char *prefix, bool on);
  void queue_battery_type_setting(uint8_t code);
  void queue_enable_disable_setting(char param_char, bool enable);
  void queue_rated_frequency_setting(uint8_t hz);
  void queue_rated_voltage_setting(uint16_t volts);
  void queue_charger_priority_setting(uint8_t code);
  void queue_clear_fault_information();
  void queue_reset_factory_setting();
  void queue_clear_power_generation();
  // Builds "^S???DAT<YYMMDDhhmmss>" + CRC + '\r'.
  void queue_inverter_datetime_setting(const ESPTime &t);
  // Reads the current time from the connected time: source and sends it to the
  // inverter. Called from the sync_inverter_datetime button. If time_source_ or
  // the time is not valid, it just logs a warning.
  void sync_inverter_datetime_now();
  void set_maximum_mains_charging_current_select(MuchgcCurrentSelect *sel) {
    this->maximum_mains_charging_current_select_ = sel;
  }
  void set_output_source_priority_select(OutputSourcePrioritySelect *sel) {
    this->output_source_priority_select_ = sel;
  }
  void set_dual_remote_switch_select(OnOffCommandSelect *sel) {
    this->dual_remote_switch_select_ = sel;
  }
  void set_ac_charging_time_start_select(AcChargingTimeStartSelect *sel) {
    this->ac_charging_time_start_select_ = sel;
  }
  void set_ac_charging_time_stop_select(AcChargingTimeStopSelect *sel) {
    this->ac_charging_time_stop_select_ = sel;
  }
  void set_ac_output_time_start_select(AcOutputTimeStartSelect *sel) {
    this->ac_output_time_start_select_ = sel;
  }
  void set_ac_output_time_stop_select(AcOutputTimeStopSelect *sel) {
    this->ac_output_time_stop_select_ = sel;
  }
  void set_output_time_start_select(OutputTimeStartSelect *sel) {
    this->output_time_start_select_ = sel;
  }
  void set_output_time_stop_select(OutputTimeStopSelect *sel) {
    this->output_time_stop_select_ = sel;
  }
  void set_battery_energy_grid_feeding_time_start_select(BatteryEnergyGridFeedingTimeStartSelect *sel) {
    this->battery_energy_grid_feeding_time_start_select_ = sel;
  }
  void set_battery_energy_grid_feeding_time_stop_select(BatteryEnergyGridFeedingTimeStopSelect *sel) {
    this->battery_energy_grid_feeding_time_stop_select_ = sel;
  }
  void set_max_charging_current_number(ThreeDigitNumber *n) {
    this->max_charging_current_number_ = n;
  }
  void set_ct_zero_power_number(ThreeDigitNumber *n) {
    this->ct_zero_power_number_ = n;
  }
  void set_grid_connected_power_number(FiveDigitNumber *n) {
    this->grid_connected_power_number_ = n;
  }
  void set_restore_second_output_battery_capacity_number(ThreeDigitNumber *n) {
    this->restore_second_output_battery_capacity_number_ = n;
  }
  void set_second_output_discharge_time_number(FourDigitNumber *n) {
    this->second_output_discharge_time_number_ = n;
  }
  void set_discharge_current_limit_number(ThreeDigitNumber *n) {
    this->discharge_current_limit_number_ = n;
  }
  void set_inverter_startup_battery_capacity_number(ThreeDigitNumber *n) {
    this->inverter_startup_battery_capacity_number_ = n;
  }
  void set_bms_lock_machine_battery_capacity_number(ThreeDigitNumber *n) {
    this->bms_lock_machine_battery_capacity_number_ = n;
  }
  void set_restore_mains_charging_battery_capacity_number(ThreeDigitNumber *n) {
    this->restore_mains_charging_battery_capacity_number_ = n;
  }
  void set_restore_battery_discharging_battery_capacity_number(ThreeDigitNumber *n) {
    this->restore_battery_discharging_battery_capacity_number_ = n;
  }
  void set_low_battery_alarm_voltage_setting_number(OneDecimalNumber *n) {
    this->low_battery_alarm_voltage_setting_number_ = n;
  }
  void set_battery_low_voltage_protection_number(OneDecimalNumber *n) {
    this->battery_low_voltage_protection_number_ = n;
  }
  void set_battery_recharge_voltage_setting_number(OneDecimalNumber *n) {
    this->battery_recharge_voltage_setting_number_ = n;
  }
  void set_battery_redischarge_voltage_setting_number(OneDecimalNumber *n) {
    this->battery_redischarge_voltage_setting_number_ = n;
  }
  void set_battery_constant_charging_voltage_setting_number(OneDecimalNumber *n) {
    this->battery_constant_charging_voltage_setting_number_ = n;
  }
  void set_battery_float_charging_voltage_setting_number(OneDecimalNumber *n) {
    this->battery_float_charging_voltage_setting_number_ = n;
  }
  void set_battery_equalization_voltage_setting_number(TwoDecimalNumber *n) {
    this->battery_equalization_voltage_setting_number_ = n;
  }
  void set_battery_equalization_time_number(ThreeDigitNumber *n) {
    this->battery_equalization_time_number_ = n;
  }
  void set_battery_equalization_timeout_number(ThreeDigitNumber *n) {
    this->battery_equalization_timeout_number_ = n;
  }
  void set_battery_equalization_interval_number(ThreeDigitNumber *n) {
    this->battery_equalization_interval_number_ = n;
  }
  void set_grid_working_range_select(GridWorkingRangeSelect *sel) {
    this->grid_working_range_select_ = sel;
  }
  void set_grid_connection_protocol_type_select(GridConnectionProtocolTypeSelect *sel) {
    this->grid_connection_protocol_type_select_ = sel;
  }
  void set_second_output_remote_activation_select(SecondOutputRemoteActivationSelect *sel) {
    this->second_output_remote_activation_select_ = sel;
  }
  void set_main_remote_power_select(MainRemotePowerSelect *sel) {
    this->main_remote_power_select_ = sel;
  }
  void set_bms_function_enable_setting_select(OnOffCommandSelect *sel) {
    this->bms_function_enable_setting_select_ = sel;
  }
  void set_battery_equalization_mode_enable_setting_select(OnOffCommandSelect *sel) {
    this->battery_equalization_mode_enable_setting_select_ = sel;
  }
  void set_battery_type_select(BatteryTypeSelect *sel) {
    this->battery_type_select_ = sel;
  }
  void set_overload_auto_restart_select(EnableDisableSelect *sel) {
    this->overload_auto_restart_select_ = sel;
  }
  void set_over_temp_auto_restart_select(EnableDisableSelect *sel) {
    this->over_temp_auto_restart_select_ = sel;
  }
  void set_buzzer_on_select(EnableDisableSelect *sel) { this->buzzer_on_select_ = sel; }
  void set_backlight_on_select(EnableDisableSelect *sel) { this->backlight_on_select_ = sel; }
  void set_display_auto_return_homepage_select(EnableDisableSelect *sel) {
    this->display_auto_return_homepage_select_ = sel;
  }
  void set_input_source_detection_prompt_sound_select(EnableDisableSelect *sel) {
    this->input_source_detection_prompt_sound_select_ = sel;
  }
  // Note: for eco_mode (PDj/PEj) we don't yet know the readback field in HEEP1.
  void set_eco_mode_select(EnableDisableSelect *sel) {
    this->eco_mode_select_ = sel;
  }
  // Note: for fault_information_storage (PDz/PEz) we don't yet know the readback
  // field. The pointer is kept mainly to add the readback in the future.
  void set_fault_information_storage_select(EnableDisableSelect *sel) {
    this->fault_information_storage_select_ = sel;
  }
  void set_rated_frequency_select(RatedFrequencySelect *sel) {
    this->rated_frequency_select_ = sel;
  }
  void set_rated_voltage_select(RatedVoltageSelect *sel) {
    this->rated_voltage_select_ = sel;
  }
  void set_charger_priority_select(ChargerPrioritySelect *sel) {
    this->charger_priority_select_ = sel;
  }

  void set_time_source(time::RealTimeClock *t) { this->time_source_ = t; }

  void set_command_throttle_ms(uint32_t ms) { this->command_throttle_ms_ = ms; }
  void set_response_timeout_ms(uint32_t ms) { this->response_timeout_ms_ = ms; }

 protected:
  struct PendingWrite {
    std::vector<uint8_t> frame;
    std::string label;
  };

  static constexpr size_t QUERY_COUNT = 18;
  static constexpr size_t MAX_QUERY_FIELDS = 20;

  static const char *query_name_(ExpectedResponse kind);
  static bool is_valid_muchgc_current_(uint16_t amps);
  void enqueue_query_(ExpectedResponse kind);
  void enqueue_write_(PendingWrite write);
  bool has_pending_query_(ExpectedResponse kind) const;
  const std::string &command_for_(ExpectedResponse kind) const;
  void try_send_next_query_();
  void publish_numeric_(ExpectedResponse kind, uint8_t index, const std::vector<std::string> &parts, float scale = 1.0f,
                        bool round_to_accuracy = true);
  void publish_text_(ExpectedResponse kind, uint8_t index, const std::vector<std::string> &parts);
  // Helper for the readback of Enable/Disable selects. Handles both composite
  // fields (offset = character position in parts[idx]) and standalone
  // single-character fields (offset = 0).
  void publish_enable_disable_(EnableDisableSelect *sel, const std::string &field, size_t offset);
  void process_line_(const std::string &raw);
  void process_hop_line_(const std::vector<std::string> &parts);
  void process_hgrid_line_(const std::vector<std::string> &parts);
  void process_hbat_line_(const std::vector<std::string> &parts);
  void process_hsts_line_(const std::vector<std::string> &parts);
  void process_hpv_line_(const std::vector<std::string> &parts);
  void process_htemp_line_(const std::vector<std::string> &parts);
  void process_heep1_line_(const std::vector<std::string> &parts);
  void process_hbms1_line_(const std::vector<std::string> &parts);
  void process_hgen_line_(const std::vector<std::string> &parts);
  void process_hpvb_line_(const std::vector<std::string> &parts);
  void process_heep2_line_(const std::vector<std::string> &parts);
  void process_himsg1_line_(const std::vector<std::string> &parts);
  void process_hbms2_line_(const std::vector<std::string> &parts);
  void process_hbms3_line_(const std::vector<std::string> &parts);
  void process_heep3_line_(const std::vector<std::string> &parts);
  void process_hsts2_line_(const std::vector<std::string> &parts);
  void process_hctmsg1_line_(const std::vector<std::string> &parts);
  void process_qprtl_line_(const std::vector<std::string> &parts);

  uint32_t command_throttle_ms_{200};
  uint32_t response_timeout_ms_{3000};

  std::array<bool, QUERY_COUNT> query_enabled_{};
  std::array<uint32_t, QUERY_COUNT> query_interval_ms_{};
  std::array<std::string, QUERY_COUNT> query_command_{};
  std::array<std::array<sensor::Sensor *, MAX_QUERY_FIELDS>, QUERY_COUNT> numeric_sensors_{};
  std::array<std::array<text_sensor::TextSensor *, MAX_QUERY_FIELDS>, QUERY_COUNT> text_sensors_{};
  text_sensor::TextSensor *system_time_text_sensor_{nullptr};
  MuchgcCurrentSelect *maximum_mains_charging_current_select_{nullptr};
  OutputSourcePrioritySelect *output_source_priority_select_{nullptr};
  OnOffCommandSelect *dual_remote_switch_select_{nullptr};
  AcChargingTimeStartSelect *ac_charging_time_start_select_{nullptr};
  AcChargingTimeStopSelect *ac_charging_time_stop_select_{nullptr};
  AcOutputTimeStartSelect *ac_output_time_start_select_{nullptr};
  AcOutputTimeStopSelect *ac_output_time_stop_select_{nullptr};
  OutputTimeStartSelect *output_time_start_select_{nullptr};
  OutputTimeStopSelect *output_time_stop_select_{nullptr};
  BatteryEnergyGridFeedingTimeStartSelect *battery_energy_grid_feeding_time_start_select_{nullptr};
  BatteryEnergyGridFeedingTimeStopSelect *battery_energy_grid_feeding_time_stop_select_{nullptr};
  ThreeDigitNumber *max_charging_current_number_{nullptr};
  ThreeDigitNumber *ct_zero_power_number_{nullptr};
  FiveDigitNumber *grid_connected_power_number_{nullptr};
  ThreeDigitNumber *restore_second_output_battery_capacity_number_{nullptr};
  FourDigitNumber *second_output_discharge_time_number_{nullptr};
  ThreeDigitNumber *discharge_current_limit_number_{nullptr};
  ThreeDigitNumber *inverter_startup_battery_capacity_number_{nullptr};
  ThreeDigitNumber *bms_lock_machine_battery_capacity_number_{nullptr};
  ThreeDigitNumber *restore_mains_charging_battery_capacity_number_{nullptr};
  ThreeDigitNumber *restore_battery_discharging_battery_capacity_number_{nullptr};
  OneDecimalNumber *low_battery_alarm_voltage_setting_number_{nullptr};
  OneDecimalNumber *battery_low_voltage_protection_number_{nullptr};
  OneDecimalNumber *battery_recharge_voltage_setting_number_{nullptr};
  OneDecimalNumber *battery_redischarge_voltage_setting_number_{nullptr};
  OneDecimalNumber *battery_constant_charging_voltage_setting_number_{nullptr};
  OneDecimalNumber *battery_float_charging_voltage_setting_number_{nullptr};
  TwoDecimalNumber *battery_equalization_voltage_setting_number_{nullptr};
  ThreeDigitNumber *battery_equalization_time_number_{nullptr};
  ThreeDigitNumber *battery_equalization_timeout_number_{nullptr};
  ThreeDigitNumber *battery_equalization_interval_number_{nullptr};
  GridWorkingRangeSelect *grid_working_range_select_{nullptr};
  GridConnectionProtocolTypeSelect *grid_connection_protocol_type_select_{nullptr};
  SecondOutputRemoteActivationSelect *second_output_remote_activation_select_{nullptr};
  MainRemotePowerSelect *main_remote_power_select_{nullptr};
  OnOffCommandSelect *bms_function_enable_setting_select_{nullptr};
  OnOffCommandSelect *battery_equalization_mode_enable_setting_select_{nullptr};
  BatteryTypeSelect *battery_type_select_{nullptr};
  EnableDisableSelect *overload_auto_restart_select_{nullptr};
  EnableDisableSelect *over_temp_auto_restart_select_{nullptr};
  EnableDisableSelect *buzzer_on_select_{nullptr};
  EnableDisableSelect *backlight_on_select_{nullptr};
  EnableDisableSelect *display_auto_return_homepage_select_{nullptr};
  EnableDisableSelect *input_source_detection_prompt_sound_select_{nullptr};
  EnableDisableSelect *eco_mode_select_{nullptr};
  EnableDisableSelect *fault_information_storage_select_{nullptr};
  RatedFrequencySelect *rated_frequency_select_{nullptr};
  RatedVoltageSelect *rated_voltage_select_{nullptr};
  ChargerPrioritySelect *charger_priority_select_{nullptr};

  std::deque<PendingWrite> pending_writes_{};
  std::deque<ExpectedResponse> pending_queries_{};
  bool command_in_flight_{false};
  ExpectedResponse in_flight_query_{ExpectedResponse::HOP};

  uint8_t ac_charging_time_start_hour_{0};
  uint8_t ac_charging_time_stop_hour_{0};
  bool ac_charging_time_start_known_{false};
  bool ac_charging_time_stop_known_{false};
  uint8_t ac_output_time_start_hour_{0};
  uint8_t ac_output_time_stop_hour_{0};
  bool ac_output_time_start_known_{false};
  bool ac_output_time_stop_known_{false};
  uint8_t output_time_start_hour_{0};
  uint8_t output_time_stop_hour_{0};
  bool output_time_start_known_{false};
  bool output_time_stop_known_{false};
  uint8_t battery_energy_grid_feeding_time_start_hour_{0};
  uint8_t battery_energy_grid_feeding_time_stop_hour_{0};
  bool battery_energy_grid_feeding_time_start_known_{false};
  bool battery_energy_grid_feeding_time_stop_known_{false};

  std::string rx_buffer_{};
  static constexpr size_t RX_BUFFER_MAX = 512;

  time::RealTimeClock *time_source_{nullptr};
};

class MuchgcCurrentSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  static const std::array<uint16_t, 7> &allowed_values();
  static const std::array<const char *, 7> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

class OutputSourcePrioritySelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order corresponds to the value in POP00..POP03.
  static const std::array<const char *, 4> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

// Generic writable parameter with a 3-digit integer value in the format
// "<prefix>%03u". Snap/clamp is done via min/max/step in NumberTraits - these
// are set from the schema in Python (and can be overridden in YAML).
class ThreeDigitNumber : public number::Number, public Parented<EasunSmtIiiComponent> {
 public:
  void set_command_prefix(const char *prefix) { this->command_prefix_ = prefix; }

 protected:
  void control(float value) override;
  const char *command_prefix_{""};
};

// Generic writable parameter with a 4-digit integer value in the format
// "<prefix>%04u". Snap/clamp is controlled via min/max/step from NumberTraits.
class FourDigitNumber : public number::Number, public Parented<EasunSmtIiiComponent> {
 public:
  void set_command_prefix(const char *prefix) { this->command_prefix_ = prefix; }

 protected:
  void control(float value) override;
  const char *command_prefix_{""};
};

// Generic writable parameter with a 5-digit integer value in the format
// "<prefix>%05u". Snap/clamp is controlled via min/max/step from NumberTraits.
class FiveDigitNumber : public number::Number, public Parented<EasunSmtIiiComponent> {
 public:
  void set_command_prefix(const char *prefix) { this->command_prefix_ = prefix; }

 protected:
  void control(float value) override;
  const char *command_prefix_{""};
};

// Generic writable parameter in the format "<prefix>44.0". Snap/clamp, like in
// ThreeDigitNumber, is controlled via min/max/step from NumberTraits.
class OneDecimalNumber : public number::Number, public Parented<EasunSmtIiiComponent> {
 public:
  void set_command_prefix(const char *prefix) { this->command_prefix_ = prefix; }

 protected:
  void control(float value) override;
  const char *command_prefix_{""};
};

// Generic writable parameter in the format "<prefix>58.40". Snap/clamp is
// controlled via min/max/step from NumberTraits.
class TwoDecimalNumber : public number::Number, public Parented<EasunSmtIiiComponent> {
 public:
  void set_command_prefix(const char *prefix) { this->command_prefix_ = prefix; }

 protected:
  void control(float value) override;
  const char *command_prefix_{""};
};

class AcChargingTimeStartSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class AcChargingTimeStopSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class AcOutputTimeStartSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class AcOutputTimeStopSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class OutputTimeStartSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class OutputTimeStopSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class BatteryEnergyGridFeedingTimeStartSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class BatteryEnergyGridFeedingTimeStopSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 protected:
  void control(const std::string &value) override;
};

class GridWorkingRangeSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order corresponds to the value sent in PGR0N.
  static const std::array<const char *, 2> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

class GridConnectionProtocolTypeSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order corresponds to the value in ^S???RS0N (N = 1..5).
  static const std::array<const char *, 5> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

class SecondOutputRemoteActivationSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  static const std::array<const char *, 3> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

class MainRemotePowerSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  static const std::array<const char *, 2> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

class OnOffCommandSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  static const std::array<const char *, 2> &allowed_options();
  void set_command_prefix(const char *prefix) { this->command_prefix_ = prefix; }

 protected:
  void control(const std::string &value) override;
  const char *command_prefix_{""};
};

class BatteryTypeSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order corresponds to the value sent in PBT0N (0..9).
  static const std::array<const char *, 10> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

// Shared class for simple Disable/Enable parameters of the form P[DE]<c>.
// Each instance has its own parameter character (e.g. 'u' for overload, 'v' for over-temp).
class EnableDisableSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order: 0 = Disable, 1 = Enable.
  static const std::array<const char *, 2> &allowed_options();
  void set_param_char(int c) { this->param_char_ = static_cast<char>(c); }
  void set_write_only_option(bool write_only_option) { this->write_only_option_ = write_only_option; }

 protected:
  void control(const std::string &value) override;
  char param_char_{' '};
  bool write_only_option_{false};
};

class RatedFrequencySelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order: "50 Hz", "60 Hz".
  static const std::array<const char *, 2> &allowed_options();
  static const std::array<uint8_t, 2> &allowed_values();

 protected:
  void control(const std::string &value) override;
};

class RatedVoltageSelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order: "220 V", "230 V", "240 V".
  static const std::array<const char *, 3> &allowed_options();
  static const std::array<uint16_t, 3> &allowed_values();

 protected:
  void control(const std::string &value) override;
};

class ChargerPrioritySelect : public select::Select, public Parented<EasunSmtIiiComponent> {
 public:
  // Order corresponds to the value in PCP0N (0..3).
  static const std::array<const char *, 4> &allowed_options();

 protected:
  void control(const std::string &value) override;
};

// "Sync Inverter Datetime From HA" button - when pressed it sends the current
// time from the ESPHome time: component to the inverter (command "^S???DAT<YYMMDDhhmmss>").
class SyncInverterDatetimeButton : public button::Button, public Parented<EasunSmtIiiComponent> {
 protected:
  void press_action() override;
};

class ClearFaultInformationButton : public button::Button, public Parented<EasunSmtIiiComponent> {
 protected:
  void press_action() override;
};

class ResetFactorySettingButton : public button::Button, public Parented<EasunSmtIiiComponent> {
 protected:
  void press_action() override;
};

class ClearPowerGenerationButton : public button::Button, public Parented<EasunSmtIiiComponent> {
 protected:
  void press_action() override;
};

}  // namespace easun_smt_iii
}  // namespace esphome
