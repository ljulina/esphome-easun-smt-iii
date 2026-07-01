// EASUN SMT-III - ESPHome external component
//
// Monitoring and control of EASUN SMT-III (and compatible) inverters over their
// RS232 serial protocol.
//
// Project: https://github.com/ljulina/esphome-easun-smt-iii
// Author:  Ladislav Julina <ljulina@email.cz>
// License: MIT (see LICENSE)

#include "easun_smt_iii.h"
#include "easun_checksum.h"
#include "esphome/core/log.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <vector>

namespace {

void trim_trailing_dots(std::string &s) {
  while (!s.empty() && s.back() == '.') {
    s.pop_back();
  }
}

void split_paren_line(const std::string &printable, std::vector<std::string> &parts) {
  parts.clear();
  std::string current;
  for (char c : printable) {
    if (c == ' ' || c == '(' || c == ')') {
      if (!current.empty()) {
        parts.push_back(current);
        current.clear();
      }
    } else {
      current += c;
    }
  }
  if (!current.empty()) {
    parts.push_back(current);
  }
  for (auto &part : parts) {
    trim_trailing_dots(part);
  }
}

// True when the first `count` characters of `s` are all decimal digits. Used to
// sanity-check structural fields (e.g. the YYMMDD date in HGEN) so a desynced or
// merged response from another command is not parsed as if it were valid.
bool field_has_leading_digits(const std::string &s, size_t count) {
  if (s.size() < count) {
    return false;
  }
  for (size_t i = 0; i < count; i++) {
    if (!isdigit(static_cast<unsigned char>(s[i]))) {
      return false;
    }
  }
  return true;
}

bool parse_float_field(const std::string &s, float *value) {
  if (value == nullptr || s.empty()) {
    return false;
  }
  char *end = nullptr;
  const float parsed = strtof(s.c_str(), &end);
  if (end == s.c_str() || (end != nullptr && *end != '\0')) {
    return false;
  }
  *value = parsed;
  return true;
}

float round_to_decimals(float value, int8_t decimals) {
  if (decimals < 0) {
    return value;
  }
  const float factor = powf(10.0f, decimals);
  return roundf(value * factor) / factor;
}

void publish_number_state(esphome::number::Number *number, const std::vector<std::string> &parts, size_t index,
                          int8_t decimals) {
  if (number == nullptr || parts.size() <= index) {
    return;
  }
  float value;
  if (!parse_float_field(parts[index], &value)) {
    return;
  }
  number->publish_state(round_to_decimals(value, decimals));
}

bool extract_option_code3_(const std::string &value, char code[4]) {
  if (value.size() < 3) {
    return false;
  }
  code[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[0])));
  code[1] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[1])));
  code[2] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[2])));
  code[3] = '\0';
  return true;
}

bool option_matches_code3_(const std::string &value, const char *option) {
  if (option == nullptr) {
    return false;
  }
  char lhs[4];
  char rhs[4];
  if (!extract_option_code3_(value, lhs)) {
    return false;
  }
  if (!extract_option_code3_(option, rhs)) {
    return false;
  }
  return lhs[0] == rhs[0] && lhs[1] == rhs[1] && lhs[2] == rhs[2];
}

bool parse_hour_option(const std::string &value, uint8_t &hour) {
  unsigned parsed = 0;
  if (sscanf(value.c_str(), "%2u:00", &parsed) != 1) {
    return false;
  }
  if (parsed > 23) {
    return false;
  }
  hour = static_cast<uint8_t>(parsed);
  return true;
}

}  // namespace

namespace esphome {
namespace easun_smt_iii {

static const char *const TAG = "easun_smt_iii";

const std::array<uint16_t, 7> &MuchgcCurrentSelect::allowed_values() {
  static const std::array<uint16_t, 7> values{{2, 10, 20, 30, 40, 50, 60}};
  return values;
}

const std::array<const char *, 7> &MuchgcCurrentSelect::allowed_options() {
  static const std::array<const char *, 7> options{{"2 A", "10 A", "20 A", "30 A", "40 A", "50 A", "60 A"}};
  return options;
}

void MuchgcCurrentSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &values = this->allowed_values();
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_muchgc_current_setting(values[i]);
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 4> &OutputSourcePrioritySelect::allowed_options() {
  // Must match OUTPUT_SOURCE_PRIORITY_OPTIONS in __init__.py (ESPHome select options).
  static const std::array<const char *, 4> options{{"SUB - Solar - Utility - Battery",
                                                     "SBU - Solar - Battery - Utility",
                                                     "SUF - Solar - Utility - Fast",
                                                     "PEC - Priority Efficiency Center"}};
  return options;
}

void OutputSourcePrioritySelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (option_matches_code3_(value, options[i])) {
      this->parent_->queue_output_source_priority_setting(static_cast<uint8_t>(i));
      this->publish_state(options[i]);
      return;
    }
  }
}

const std::array<const char *, 2> &GridWorkingRangeSelect::allowed_options() {
  // Order corresponds to the value in PGR0N: 0 -> APL (Appliances), 1 -> UPS.
  static const std::array<const char *, 2> options{{"APL - Appliances", "UPS"}};
  return options;
}

void GridWorkingRangeSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (option_matches_code3_(value, options[i])) {
      this->parent_->queue_grid_working_range_setting(static_cast<uint8_t>(i));
      this->publish_state(options[i]);
      return;
    }
  }
}

const std::array<const char *, 5> &GridConnectionProtocolTypeSelect::allowed_options() {
  static const std::array<const char *, 5> options{{"mode1", "mode2", "mode3", "mode4", "mode5"}};
  return options;
}

void GridConnectionProtocolTypeSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_grid_connection_protocol_type_setting(static_cast<uint8_t>(i + 1));
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 3> &SecondOutputRemoteActivationSelect::allowed_options() {
  static const std::array<const char *, 3> options{{"Off", "On", "Automatic"}};
  return options;
}

void SecondOutputRemoteActivationSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_second_output_remote_activation_setting(static_cast<uint8_t>(i));
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 2> &MainRemotePowerSelect::allowed_options() {
  static const std::array<const char *, 2> options{{"Off", "On"}};
  return options;
}

void MainRemotePowerSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_main_remote_power_setting(i == 1);
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 2> &OnOffCommandSelect::allowed_options() {
  static const std::array<const char *, 2> options{{"Off", "On"}};
  return options;
}

void OnOffCommandSelect::control(const std::string &value) {
  if (this->parent_ == nullptr || this->command_prefix_ == nullptr || this->command_prefix_[0] == '\0') {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_on_off_command_setting(this->command_prefix_, i == 1);
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 10> &BatteryTypeSelect::allowed_options() {
  // Order corresponds to the value in PBT0N (0..9).
  static const std::array<const char *, 10> options{
      {"AGM", "FLD", "USE", "LIA", "PYL", "TQF", "GRO", "LIB", "LIC", "FEL"}};
  return options;
}

void BatteryTypeSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (option_matches_code3_(value, options[i])) {
      this->parent_->queue_battery_type_setting(static_cast<uint8_t>(i));
      this->publish_state(options[i]);
      return;
    }
  }
}

const std::array<const char *, 2> &EnableDisableSelect::allowed_options() {
  // Order: 0 = Disable (P D c), 1 = Enable (P E c).
  static const std::array<const char *, 2> options{{"Disable", "Enable"}};
  return options;
}

void EnableDisableSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  if (this->write_only_option_ && value == "Write Only") {
    this->publish_state(value);
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_enable_disable_setting(this->param_char_, i == 1);
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 2> &RatedFrequencySelect::allowed_options() {
  static const std::array<const char *, 2> options{{"50 Hz", "60 Hz"}};
  return options;
}

const std::array<uint8_t, 2> &RatedFrequencySelect::allowed_values() {
  static const std::array<uint8_t, 2> values{{50, 60}};
  return values;
}

void RatedFrequencySelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  const auto &values = this->allowed_values();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_rated_frequency_setting(values[i]);
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 3> &RatedVoltageSelect::allowed_options() {
  static const std::array<const char *, 3> options{{"220 V", "230 V", "240 V"}};
  return options;
}

const std::array<uint16_t, 3> &RatedVoltageSelect::allowed_values() {
  static const std::array<uint16_t, 3> values{{220, 230, 240}};
  return values;
}

void RatedVoltageSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  const auto &values = this->allowed_values();
  for (size_t i = 0; i < options.size(); i++) {
    if (value == options[i]) {
      this->parent_->queue_rated_voltage_setting(values[i]);
      this->publish_state(value);
      return;
    }
  }
}

const std::array<const char *, 4> &ChargerPrioritySelect::allowed_options() {
  // Order corresponds to the value in PCP0N (0..3).
  static const std::array<const char *, 4> options{
      {"CSO - Solar First", "SNU - Solar and Utility", "OSO - Only Solar", "SOR - Solar Residual"}};
  return options;
}

void ChargerPrioritySelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  const auto &options = this->allowed_options();
  for (size_t i = 0; i < options.size(); i++) {
    if (option_matches_code3_(value, options[i])) {
      this->parent_->queue_charger_priority_setting(static_cast<uint8_t>(i));
      this->publish_state(options[i]);
      return;
    }
  }
}

void ThreeDigitNumber::control(float value) {
  if (this->parent_ == nullptr || this->command_prefix_ == nullptr || this->command_prefix_[0] == '\0') {
    return;
  }
  // Clamp + snap to the step from traits (min/max/step come from the schema in
  // Python, optionally overridden by YAML).
  const float min_v = this->traits.get_min_value();
  const float max_v = this->traits.get_max_value();
  const float step = this->traits.get_step();
  if (value < min_v) value = min_v;
  if (value > max_v) value = max_v;
  if (step > 0.0f) {
    const float snapped = min_v + roundf((value - min_v) / step) * step;
    value = snapped;
    if (value < min_v) value = min_v;
    if (value > max_v) value = max_v;
  }
  const auto int_val = static_cast<uint16_t>(lroundf(value));
  this->parent_->queue_three_digit_setting(this->command_prefix_, int_val);
  this->publish_state(static_cast<float>(int_val));
}

void FourDigitNumber::control(float value) {
  if (this->parent_ == nullptr || this->command_prefix_ == nullptr || this->command_prefix_[0] == '\0') {
    return;
  }
  const float min_v = this->traits.get_min_value();
  const float max_v = this->traits.get_max_value();
  const float step = this->traits.get_step();
  if (value < min_v) value = min_v;
  if (value > max_v) value = max_v;
  if (step > 0.0f) {
    const float snapped = min_v + roundf((value - min_v) / step) * step;
    value = snapped;
    if (value < min_v) value = min_v;
    if (value > max_v) value = max_v;
  }
  const auto int_val = static_cast<uint16_t>(lroundf(value));
  this->parent_->queue_four_digit_setting(this->command_prefix_, int_val);
  this->publish_state(static_cast<float>(int_val));
}

void FiveDigitNumber::control(float value) {
  if (this->parent_ == nullptr || this->command_prefix_ == nullptr || this->command_prefix_[0] == '\0') {
    return;
  }
  const float min_v = this->traits.get_min_value();
  const float max_v = this->traits.get_max_value();
  const float step = this->traits.get_step();
  if (value < min_v) value = min_v;
  if (value > max_v) value = max_v;
  if (step > 0.0f) {
    const float snapped = min_v + roundf((value - min_v) / step) * step;
    value = snapped;
    if (value < min_v) value = min_v;
    if (value > max_v) value = max_v;
  }
  const auto int_val = static_cast<uint16_t>(lroundf(value));
  this->parent_->queue_five_digit_setting(this->command_prefix_, int_val);
  this->publish_state(static_cast<float>(int_val));
}

void OneDecimalNumber::control(float value) {
  if (this->parent_ == nullptr || this->command_prefix_ == nullptr || this->command_prefix_[0] == '\0') {
    return;
  }
  const float min_v = this->traits.get_min_value();
  const float max_v = this->traits.get_max_value();
  const float step = this->traits.get_step();
  if (value < min_v) value = min_v;
  if (value > max_v) value = max_v;
  if (step > 0.0f) {
    const float snapped = min_v + roundf((value - min_v) / step) * step;
    value = snapped;
    if (value < min_v) value = min_v;
    if (value > max_v) value = max_v;
  }
  const auto tenths = static_cast<uint16_t>(lroundf(value * 10.0f));
  this->parent_->queue_one_decimal_setting(this->command_prefix_, tenths);
  this->publish_state(static_cast<float>(tenths) / 10.0f);
}

void TwoDecimalNumber::control(float value) {
  if (this->parent_ == nullptr || this->command_prefix_ == nullptr || this->command_prefix_[0] == '\0') {
    return;
  }
  const float min_v = this->traits.get_min_value();
  const float max_v = this->traits.get_max_value();
  const float step = this->traits.get_step();
  if (value < min_v) value = min_v;
  if (value > max_v) value = max_v;
  if (step > 0.0f) {
    const float snapped = min_v + roundf((value - min_v) / step) * step;
    value = snapped;
    if (value < min_v) value = min_v;
    if (value > max_v) value = max_v;
  }
  const auto hundredths = static_cast<uint16_t>(lroundf(value * 100.0f));
  this->parent_->queue_two_decimal_setting(this->command_prefix_, hundredths);
  this->publish_state(static_cast<float>(hundredths) / 100.0f);
}

void AcChargingTimeStartSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_ac_charging_time_start_setting(hour);
  this->publish_state(value);
}

void AcChargingTimeStopSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_ac_charging_time_stop_setting(hour);
  this->publish_state(value);
}

void AcOutputTimeStartSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_ac_output_time_start_setting(hour);
  this->publish_state(value);
}

void AcOutputTimeStopSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_ac_output_time_stop_setting(hour);
  this->publish_state(value);
}

void OutputTimeStartSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_output_time_start_setting(hour);
  this->publish_state(value);
}

void OutputTimeStopSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_output_time_stop_setting(hour);
  this->publish_state(value);
}

void BatteryEnergyGridFeedingTimeStartSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_battery_energy_grid_feeding_time_start_setting(hour);
  this->publish_state(value);
}

void BatteryEnergyGridFeedingTimeStopSelect::control(const std::string &value) {
  if (this->parent_ == nullptr) {
    return;
  }
  uint8_t hour = 0;
  if (!parse_hour_option(value, hour)) {
    return;
  }
  this->parent_->queue_battery_energy_grid_feeding_time_stop_setting(hour);
  this->publish_state(value);
}

EasunSmtIiiComponent::EasunSmtIiiComponent() {
  this->query_interval_ms_.fill(10000);
  this->query_command_[static_cast<size_t>(ExpectedResponse::HOP)] = "HOP\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HGRID)] = "HGRID\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HBAT)] = "HBAT\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HSTS)] = "HSTS\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HPV)] = "HPV\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HTEMP)] = "HTEMP\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HEEP1)] = "HEEP1\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HBMS1)] = "HBMS1\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HGEN)] = "HGEN\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HPVB)] = "HPVB\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HEEP2)] = "HEEP2\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HIMSG1)] = "HIMSG1\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HBMS2)] = "HBMS2\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HBMS3)] = "HBMS3\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HEEP3)] = "HEEP3\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HSTS2)] = "HSTS2\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::HCTMSG1)] = "HCTMSG1\r";
  this->query_command_[static_cast<size_t>(ExpectedResponse::QPRTL)] = "QPRTL\r";
}

const char *EasunSmtIiiComponent::query_name_(ExpectedResponse kind) {
  switch (kind) {
    case ExpectedResponse::HOP:
      return "hop";
    case ExpectedResponse::HGRID:
      return "hgrid";
    case ExpectedResponse::HBAT:
      return "hbat";
    case ExpectedResponse::HSTS:
      return "hsts";
    case ExpectedResponse::HPV:
      return "hpv";
    case ExpectedResponse::HTEMP:
      return "htemp";
    case ExpectedResponse::HEEP1:
      return "heep1";
    case ExpectedResponse::HBMS1:
      return "hbms1";
    case ExpectedResponse::HGEN:
      return "hgen";
    case ExpectedResponse::HPVB:
      return "hpvb";
    case ExpectedResponse::HEEP2:
      return "heep2";
    case ExpectedResponse::HIMSG1:
      return "himsg1";
    case ExpectedResponse::HBMS2:
      return "hbms2";
    case ExpectedResponse::HBMS3:
      return "hbms3";
    case ExpectedResponse::HEEP3:
      return "heep3";
    case ExpectedResponse::HSTS2:
      return "hsts2";
    case ExpectedResponse::HCTMSG1:
      return "hctmsg1";
    case ExpectedResponse::QPRTL:
      return "qprtl";
  }
  return "unknown";
}

bool EasunSmtIiiComponent::is_valid_muchgc_current_(uint16_t amps) {
  for (const auto value : MuchgcCurrentSelect::allowed_values()) {
    if (value == amps) {
      return true;
    }
  }
  return false;
}


void EasunSmtIiiComponent::set_query_enabled(uint8_t kind, bool enabled) {
  if (kind >= QUERY_COUNT) {
    return;
  }
  this->query_enabled_[kind] = enabled;
}

void EasunSmtIiiComponent::set_query_interval_ms(uint8_t kind, uint32_t ms) {
  if (kind >= QUERY_COUNT) {
    return;
  }
  this->query_interval_ms_[kind] = ms;
}

void EasunSmtIiiComponent::set_query_command(uint8_t kind, const std::string &cmd) {
  if (kind >= QUERY_COUNT) {
    return;
  }
  this->query_command_[kind] = cmd;
}

void EasunSmtIiiComponent::set_numeric_sensor(uint8_t kind, uint8_t index, sensor::Sensor *s) {
  if (kind >= QUERY_COUNT || index >= MAX_QUERY_FIELDS) {
    return;
  }
  this->numeric_sensors_[kind][index] = s;
}

void EasunSmtIiiComponent::set_text_sensor(uint8_t kind, uint8_t index, text_sensor::TextSensor *s) {
  if (kind >= QUERY_COUNT || index >= MAX_QUERY_FIELDS) {
    return;
  }
  this->text_sensors_[kind][index] = s;
}

void EasunSmtIiiComponent::queue_muchgc_current_setting(uint16_t amps) {
  if (!this->is_valid_muchgc_current_(amps)) {
    ESP_LOGW(TAG, "MUCHGC: unsupported current %u A", amps);
    return;
  }

  char payload[16];
  snprintf(payload, sizeof(payload), "MUCHGC%03u", amps);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_output_source_priority_setting(uint8_t code) {
  if (code > 3) {
    ESP_LOGW(TAG, "POP: unsupported code %u (allowed 0..3)", code);
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "POP%02u", code);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_three_digit_setting(const char *prefix, uint16_t value) {
  if (prefix == nullptr || prefix[0] == '\0') {
    ESP_LOGW(TAG, "queue_three_digit_setting: empty prefix");
    return;
  }
  if (value > 999) {
    ESP_LOGW(TAG, "%s: value %u out of 3-digit range", prefix, value);
    return;
  }

  char payload[24];
  const int written = snprintf(payload, sizeof(payload), "%s%03u", prefix, value);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "queue_three_digit_setting: payload overflow (prefix=%s)", prefix);
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_four_digit_setting(const char *prefix, uint16_t value) {
  if (prefix == nullptr || prefix[0] == '\0') {
    ESP_LOGW(TAG, "queue_four_digit_setting: empty prefix");
    return;
  }
  if (value > 9999) {
    ESP_LOGW(TAG, "%s: value %u out of 4-digit range", prefix, value);
    return;
  }

  char payload[24];
  const int written = snprintf(payload, sizeof(payload), "%s%04u", prefix, value);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "queue_four_digit_setting: payload overflow (prefix=%s)", prefix);
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_five_digit_setting(const char *prefix, uint16_t value) {
  if (prefix == nullptr || prefix[0] == '\0') {
    ESP_LOGW(TAG, "queue_five_digit_setting: empty prefix");
    return;
  }

  char payload[24];
  const int written = snprintf(payload, sizeof(payload), "%s%05u", prefix, value);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "queue_five_digit_setting: payload overflow (prefix=%s)", prefix);
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_one_decimal_setting(const char *prefix, uint16_t value_tenths) {
  if (prefix == nullptr || prefix[0] == '\0') {
    ESP_LOGW(TAG, "queue_one_decimal_setting: empty prefix");
    return;
  }

  char payload[24];
  const int whole = value_tenths / 10;
  const int tenth = value_tenths % 10;
  const int written = snprintf(payload, sizeof(payload), "%s%d.%d", prefix, whole, tenth);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "queue_one_decimal_setting: payload overflow (prefix=%s)", prefix);
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_two_decimal_setting(const char *prefix, uint16_t value_hundredths) {
  if (prefix == nullptr || prefix[0] == '\0') {
    ESP_LOGW(TAG, "queue_two_decimal_setting: empty prefix");
    return;
  }

  char payload[24];
  const int whole = value_hundredths / 100;
  const int hundredths = value_hundredths % 100;
  const int written = snprintf(payload, sizeof(payload), "%s%d.%02d", prefix, whole, hundredths);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "queue_two_decimal_setting: payload overflow (prefix=%s)", prefix);
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_ac_charging_time_setting(uint8_t start_hour, uint8_t end_hour) {
  if (start_hour > 23 || end_hour > 23) {
    ESP_LOGW(TAG, "ACCT: unsupported range %u-%u (allowed 0..23)", start_hour, end_hour);
    return;
  }

  char payload[32];
  const int written = snprintf(payload, sizeof(payload), "^S???ACCT%02u00-%02u00x", start_hour, end_hour);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "ACCT: payload overflow");
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_ac_output_time_setting(uint8_t start_hour, uint8_t end_hour) {
  if (start_hour > 23 || end_hour > 23) {
    ESP_LOGW(TAG, "AC output time: unsupported range %u-%u (allowed 0..23)", start_hour, end_hour);
    return;
  }
  ESP_LOGW(TAG, "AC output time write command is not known yet; prepared value would be %02u%02u",
           start_hour, end_hour);
}

void EasunSmtIiiComponent::queue_output_time_setting(uint8_t start_hour, uint8_t end_hour) {
  if (start_hour > 23 || end_hour > 23) {
    ESP_LOGW(TAG, "DALT: unsupported range %u-%u (allowed 0..23)", start_hour, end_hour);
    return;
  }

  char payload[32];
  const int written = snprintf(payload, sizeof(payload), "^S???DALT%02u00-%02u00", start_hour, end_hour);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "DALT: payload overflow");
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_battery_energy_grid_feeding_time_setting(uint8_t start_hour, uint8_t end_hour) {
  if (start_hour > 23 || end_hour > 23) {
    ESP_LOGW(TAG, "PLST: unsupported range %u-%u (allowed 0..23)", start_hour, end_hour);
    return;
  }

  char payload[32];
  const int written = snprintf(payload, sizeof(payload), "^S???PLST%02u00-%02u00m", start_hour, end_hour);
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "PLST: payload overflow");
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_ac_charging_time_start_setting(uint8_t start_hour) {
  this->ac_charging_time_start_hour_ = start_hour;
  this->ac_charging_time_start_known_ = true;
  if (!this->ac_charging_time_stop_known_) {
    ESP_LOGW(TAG, "ACCT start set but stop hour unknown yet; set stop too or wait for readback");
    return;
  }
  this->queue_ac_charging_time_setting(this->ac_charging_time_start_hour_, this->ac_charging_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_ac_charging_time_stop_setting(uint8_t stop_hour) {
  this->ac_charging_time_stop_hour_ = stop_hour;
  this->ac_charging_time_stop_known_ = true;
  if (!this->ac_charging_time_start_known_) {
    ESP_LOGW(TAG, "ACCT stop set but start hour unknown yet; set start too or wait for readback");
    return;
  }
  this->queue_ac_charging_time_setting(this->ac_charging_time_start_hour_, this->ac_charging_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_ac_output_time_start_setting(uint8_t start_hour) {
  this->ac_output_time_start_hour_ = start_hour;
  this->ac_output_time_start_known_ = true;
  if (!this->ac_output_time_stop_known_) {
    ESP_LOGW(TAG, "AC output time start set but stop hour unknown yet; set stop too or wait for readback");
    return;
  }
  this->queue_ac_output_time_setting(this->ac_output_time_start_hour_, this->ac_output_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_ac_output_time_stop_setting(uint8_t stop_hour) {
  this->ac_output_time_stop_hour_ = stop_hour;
  this->ac_output_time_stop_known_ = true;
  if (!this->ac_output_time_start_known_) {
    ESP_LOGW(TAG, "AC output time stop set but start hour unknown yet; set start too or wait for readback");
    return;
  }
  this->queue_ac_output_time_setting(this->ac_output_time_start_hour_, this->ac_output_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_output_time_start_setting(uint8_t start_hour) {
  this->output_time_start_hour_ = start_hour;
  this->output_time_start_known_ = true;
  if (!this->output_time_stop_known_) {
    ESP_LOGW(TAG, "DALT start set but stop hour unknown yet; set stop too or wait for readback");
    return;
  }
  this->queue_output_time_setting(this->output_time_start_hour_, this->output_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_output_time_stop_setting(uint8_t stop_hour) {
  this->output_time_stop_hour_ = stop_hour;
  this->output_time_stop_known_ = true;
  if (!this->output_time_start_known_) {
    ESP_LOGW(TAG, "DALT stop set but start hour unknown yet; set start too or wait for readback");
    return;
  }
  this->queue_output_time_setting(this->output_time_start_hour_, this->output_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_battery_energy_grid_feeding_time_start_setting(uint8_t start_hour) {
  this->battery_energy_grid_feeding_time_start_hour_ = start_hour;
  this->battery_energy_grid_feeding_time_start_known_ = true;
  if (!this->battery_energy_grid_feeding_time_stop_known_) {
    ESP_LOGW(TAG, "PLST start set but stop hour unknown yet; set stop too or wait for readback");
    return;
  }
  this->queue_battery_energy_grid_feeding_time_setting(
      this->battery_energy_grid_feeding_time_start_hour_, this->battery_energy_grid_feeding_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_battery_energy_grid_feeding_time_stop_setting(uint8_t stop_hour) {
  this->battery_energy_grid_feeding_time_stop_hour_ = stop_hour;
  this->battery_energy_grid_feeding_time_stop_known_ = true;
  if (!this->battery_energy_grid_feeding_time_start_known_) {
    ESP_LOGW(TAG, "PLST stop set but start hour unknown yet; set start too or wait for readback");
    return;
  }
  this->queue_battery_energy_grid_feeding_time_setting(
      this->battery_energy_grid_feeding_time_start_hour_, this->battery_energy_grid_feeding_time_stop_hour_);
}

void EasunSmtIiiComponent::queue_grid_working_range_setting(uint8_t code) {
  if (code > 1) {
    ESP_LOGW(TAG, "PGR: unsupported code %u (allowed 0=Appliances, 1=UPS)", code);
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "PGR0%u", code);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_grid_connection_protocol_type_setting(uint8_t mode) {
  if (mode < 1 || mode > 5) {
    ESP_LOGW(TAG, "RS: unsupported mode %u (allowed 1..5)", mode);
    return;
  }

  char payload[16];
  snprintf(payload, sizeof(payload), "^S???RS%02u", mode);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_second_output_remote_activation_setting(uint8_t mode) {
  if (mode > 2) {
    ESP_LOGW(TAG, "DAULREMOTESW: unsupported mode %u (allowed 0..2)", mode);
    return;
  }

  char payload[24];
  snprintf(payload, sizeof(payload), "DAULREMOTESW%u", mode);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_main_remote_power_setting(bool on) {
  const char *payload = on ? "MAINREMOTESW1" : "MAINREMOTESW0";
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_on_off_command_setting(const char *prefix, bool on) {
  if (prefix == nullptr || prefix[0] == '\0') {
    ESP_LOGW(TAG, "queue_on_off_command_setting: empty prefix");
    return;
  }

  char payload[24];
  const int written = snprintf(payload, sizeof(payload), "%s%c", prefix, on ? '1' : '0');
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "queue_on_off_command_setting: payload overflow (prefix=%s)", prefix);
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_battery_type_setting(uint8_t code) {
  if (code > 9) {
    ESP_LOGW(TAG, "PBT: unsupported code %u (allowed 0..9)", code);
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "PBT0%u", code);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_enable_disable_setting(char param_char, bool enable) {
  if (param_char < 'a' || param_char > 'z') {
    ESP_LOGW(TAG, "P[DE]: invalid param char 0x%02X", static_cast<uint8_t>(param_char));
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "P%c%c", enable ? 'E' : 'D', param_char);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_rated_frequency_setting(uint8_t hz) {
  if (hz != 50 && hz != 60) {
    ESP_LOGW(TAG, "F: unsupported frequency %u Hz (allowed 50, 60)", hz);
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "F%u", hz);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_rated_voltage_setting(uint16_t volts) {
  if (volts != 220 && volts != 230 && volts != 240) {
    ESP_LOGW(TAG, "V: unsupported voltage %u V (allowed 220, 230, 240)", volts);
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "V%03u", volts);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_charger_priority_setting(uint8_t code) {
  if (code > 3) {
    ESP_LOGW(TAG, "PCP: unsupported code %u (allowed 0..3)", code);
    return;
  }

  char payload[8];
  snprintf(payload, sizeof(payload), "PCP0%u", code);
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_clear_fault_information() {
  const char *payload = "FAULTCG";
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_reset_factory_setting() {
  const char *payload = "PF";
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_clear_power_generation() {
  const char *payload = "^S???CLE";
  const auto payload_len = strlen(payload);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::queue_inverter_datetime_setting(const ESPTime &t) {
  // Payload: "^S???DAT" + YYMMDDhhmmss (year = last 2 digits, each unit 2 chars).
  // 20 payload characters in total.
  char payload[24];
  const unsigned yy = static_cast<unsigned>(t.year) % 100u;
  const int written = snprintf(payload, sizeof(payload), "^S???DAT%02u%02u%02u%02u%02u%02u",
                               yy, static_cast<unsigned>(t.month),
                               static_cast<unsigned>(t.day_of_month),
                               static_cast<unsigned>(t.hour),
                               static_cast<unsigned>(t.minute),
                               static_cast<unsigned>(t.second));
  if (written <= 0 || static_cast<size_t>(written) >= sizeof(payload)) {
    ESP_LOGW(TAG, "DAT: payload overflow");
    return;
  }
  const auto payload_len = static_cast<size_t>(written);
  const auto crc = checksum_encode(reinterpret_cast<const uint8_t *>(payload), payload_len);

  PendingWrite write;
  write.label = payload;
  write.frame.reserve(payload_len + 3);
  write.frame.insert(write.frame.end(), payload, payload + payload_len);
  write.frame.push_back(static_cast<uint8_t>((crc >> 8) & 0xFF));
  write.frame.push_back(static_cast<uint8_t>(crc & 0xFF));
  write.frame.push_back('\r');

  this->enqueue_write_(std::move(write));
}

void EasunSmtIiiComponent::sync_inverter_datetime_now() {
  if (this->time_source_ == nullptr) {
    ESP_LOGW(TAG, "sync_inverter_datetime: time: source not configured (set time_id in YAML)");
    return;
  }
  const ESPTime now = this->time_source_->now();
  if (!now.is_valid()) {
    ESP_LOGW(TAG, "sync_inverter_datetime: time not valid yet (SNTP still syncing?)");
    return;
  }
  ESP_LOGI(TAG, "sync_inverter_datetime: sending %04u-%02u-%02u %02u:%02u:%02u",
           static_cast<unsigned>(now.year), static_cast<unsigned>(now.month),
           static_cast<unsigned>(now.day_of_month), static_cast<unsigned>(now.hour),
           static_cast<unsigned>(now.minute), static_cast<unsigned>(now.second));
  this->queue_inverter_datetime_setting(now);
}

void SyncInverterDatetimeButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->sync_inverter_datetime_now();
  }
}

void ClearFaultInformationButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->queue_clear_fault_information();
  }
}

void ResetFactorySettingButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->queue_reset_factory_setting();
  }
}

void ClearPowerGenerationButton::press_action() {
  if (this->parent_ != nullptr) {
    this->parent_->queue_clear_power_generation();
  }
}

bool EasunSmtIiiComponent::has_pending_query_(ExpectedResponse kind) const {
  if (this->command_in_flight_ && this->in_flight_query_ == kind) {
    return true;
  }
  for (const auto &pending : this->pending_queries_) {
    if (pending == kind) {
      return true;
    }
  }
  return false;
}

const std::string &EasunSmtIiiComponent::command_for_(ExpectedResponse kind) const {
  return this->query_command_[static_cast<size_t>(kind)];
}

void EasunSmtIiiComponent::enqueue_query_(ExpectedResponse kind) {
  if (this->has_pending_query_(kind)) {
    return;
  }
  this->pending_queries_.push_back(kind);
  this->try_send_next_query_();
}

void EasunSmtIiiComponent::enqueue_write_(PendingWrite write) {
  this->pending_writes_.push_back(std::move(write));
  this->try_send_next_query_();
}

void EasunSmtIiiComponent::try_send_next_query_() {
  if (this->command_in_flight_) {
    return;
  }

  if (!this->pending_writes_.empty()) {
    auto write = std::move(this->pending_writes_.front());
    this->pending_writes_.pop_front();
    ESP_LOGD(TAG, "Sending write command %s", write.label.c_str());
    this->write_array(write.frame.data(), write.frame.size());
    this->set_timeout("command_throttle", this->command_throttle_ms_, [this]() { this->try_send_next_query_(); });
    return;
  }

  if (this->pending_queries_.empty()) {
    return;
  }

  const auto kind = this->pending_queries_.front();
  this->pending_queries_.pop_front();

  this->command_in_flight_ = true;
  this->in_flight_query_ = kind;
  this->write_str(this->command_for_(kind).c_str());

  this->set_timeout("response_timeout", this->response_timeout_ms_, [this, kind]() {
    if (!this->command_in_flight_ || this->in_flight_query_ != kind) {
      return;
    }
    ESP_LOGW(TAG, "Timeout waiting for response to %s", this->query_name_(kind));
    this->command_in_flight_ = false;
    this->set_timeout("command_throttle", this->command_throttle_ms_, [this]() { this->try_send_next_query_(); });
  });
}

void EasunSmtIiiComponent::setup() {
  this->rx_buffer_.reserve(64);
  if (this->dropped_response_count_sensor_ != nullptr) {
    this->dropped_response_count_sensor_->publish_state(this->dropped_response_count_);
  }
  // Register poll intervals in an order that separates similarly-shaped
  // responses when several intervals fire at the same time. This does not
  // change entity IDs or YAML keys; it only affects the enqueue order.
  static constexpr std::array<ExpectedResponse, QUERY_COUNT> poll_order{{
      ExpectedResponse::HOP,     ExpectedResponse::HGRID,  ExpectedResponse::HTEMP,
      ExpectedResponse::HPV,     ExpectedResponse::HBAT,   ExpectedResponse::HSTS,
      ExpectedResponse::HGEN,    ExpectedResponse::HPVB,   ExpectedResponse::HBMS1,
      ExpectedResponse::HIMSG1,  ExpectedResponse::HEEP1,  ExpectedResponse::HSTS2,
      ExpectedResponse::HEEP3,   ExpectedResponse::HCTMSG1, ExpectedResponse::HEEP2,
      ExpectedResponse::QPRTL,   ExpectedResponse::HBMS2,  ExpectedResponse::HBMS3,
  }};
  for (const auto kind : poll_order) {
    const uint8_t i = static_cast<uint8_t>(kind);
    if (!this->query_enabled_[i] || this->query_interval_ms_[i] == 0 || this->query_command_[i].empty()) {
      continue;
    }
    auto interval_name = std::string("poll_") + this->query_name_(kind);
    this->set_interval(interval_name, this->query_interval_ms_[i], [this, kind]() { this->enqueue_query_(kind); });
  }
}

void EasunSmtIiiComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "EASUN SMT-III");
  ESP_LOGCONFIG(TAG, "  RX routing: one command in flight, matched by sent query");
  ESP_LOGCONFIG(TAG, "  Command throttle: %ums", this->command_throttle_ms_);
  ESP_LOGCONFIG(TAG, "  Response timeout: %ums", this->response_timeout_ms_);
  for (uint8_t i = 0; i < QUERY_COUNT; i++) {
    if (!this->query_enabled_[i]) {
      continue;
    }
    auto kind = static_cast<ExpectedResponse>(i);
    ESP_LOGCONFIG(TAG, "  %s interval: %ums", this->query_name_(kind), this->query_interval_ms_[i]);
    ESP_LOGCONFIG(TAG, "  %s command: %s", this->query_name_(kind), this->query_command_[i].c_str());
  }
}

void EasunSmtIiiComponent::loop() {
  while (this->available()) {
    uint8_t b;
    if (!this->read_byte(&b)) {
      break;
    }
    if (b == '\r' || b == '\n') {
      if (!this->rx_buffer_.empty()) {
        this->process_line_(this->rx_buffer_);
        this->rx_buffer_.clear();
      }
      continue;
    }
    this->rx_buffer_.push_back(static_cast<char>(b));
    if (this->rx_buffer_.size() >= RX_BUFFER_MAX) {
      ESP_LOGW(TAG, "RX buffer overflow, clearing");
      this->rx_buffer_.clear();
    }
  }
}

void EasunSmtIiiComponent::publish_numeric_(ExpectedResponse kind, uint8_t index, const std::vector<std::string> &parts,
                                            float scale, bool round_to_accuracy) {
  if (index >= MAX_QUERY_FIELDS || index >= parts.size()) {
    return;
  }
  auto *sensor = this->numeric_sensors_[static_cast<size_t>(kind)][index];
  if (sensor == nullptr) {
    return;
  }
  float raw_value;
  if (!parse_float_field(parts[index], &raw_value)) {
    this->record_dropped_response_(std::string(this->query_name_(kind)) + "[" + std::to_string(index) +
                                   "]: non-numeric field '" + parts[index] + "', dropping value");
    return;
  }
  float value = raw_value * scale;
  const int8_t decimals = round_to_accuracy ? sensor->get_accuracy_decimals() : -1;
  value = round_to_decimals(value, decimals);
  sensor->publish_state(value);
}

void EasunSmtIiiComponent::publish_text_(ExpectedResponse kind, uint8_t index, const std::vector<std::string> &parts) {
  if (index >= MAX_QUERY_FIELDS || index >= parts.size()) {
    return;
  }
  auto *sensor = this->text_sensors_[static_cast<size_t>(kind)][index];
  if (sensor == nullptr) {
    return;
  }
  sensor->publish_state(parts[index]);
}

void EasunSmtIiiComponent::record_dropped_response_(const std::string &reason) {
  ESP_LOGW(TAG, "%s", reason.c_str());
  this->dropped_response_count_++;
  if (this->dropped_response_count_sensor_ != nullptr) {
    this->dropped_response_count_sensor_->publish_state(this->dropped_response_count_);
  }
  if (this->dropped_response_last_text_sensor_ != nullptr) {
    this->dropped_response_last_text_sensor_->publish_state(reason);
  }
}

void EasunSmtIiiComponent::publish_enable_disable_(EnableDisableSelect *sel, const std::string &field, size_t offset) {
  if (sel == nullptr || field.size() <= offset) {
    return;
  }
  const char c = field[offset];
  if (c < '0' || c > '1') {
    return;
  }
  const auto &options = EnableDisableSelect::allowed_options();
  sel->publish_state(options[c - '0']);
}

void EasunSmtIiiComponent::process_line_(const std::string &raw) {
  std::string printable;
  printable.reserve(raw.size());
  for (char c : raw) {
    if (c >= 32 && c <= 126) {
      printable += c;
    }
  }
  if (printable.empty() || printable[0] != '(') {
    return;
  }

  if (!this->command_in_flight_) {
    ESP_LOGW(TAG, "RX line but no command in flight; ignored");
    return;
  }

  this->cancel_timeout("response_timeout");

  const ExpectedResponse kind = this->in_flight_query_;
  this->command_in_flight_ = false;

  std::vector<std::string> parts;
  split_paren_line(printable, parts);

  switch (kind) {
    case ExpectedResponse::HOP:
      this->process_hop_line_(parts);
      break;
    case ExpectedResponse::HGRID:
      this->process_hgrid_line_(parts);
      break;
    case ExpectedResponse::HBAT:
      this->process_hbat_line_(parts);
      break;
    case ExpectedResponse::HSTS:
      this->process_hsts_line_(parts);
      break;
    case ExpectedResponse::HPV:
      this->process_hpv_line_(parts);
      break;
    case ExpectedResponse::HTEMP:
      this->process_htemp_line_(parts);
      break;
    case ExpectedResponse::HEEP1:
      this->process_heep1_line_(parts);
      break;
    case ExpectedResponse::HBMS1:
      this->process_hbms1_line_(parts);
      break;
    case ExpectedResponse::HGEN:
      this->process_hgen_line_(parts);
      break;
    case ExpectedResponse::HPVB:
      this->process_hpvb_line_(parts);
      break;
    case ExpectedResponse::HEEP2:
      this->process_heep2_line_(parts);
      break;
    case ExpectedResponse::HIMSG1:
      this->process_himsg1_line_(parts);
      break;
    case ExpectedResponse::HBMS2:
      this->process_hbms2_line_(parts);
      break;
    case ExpectedResponse::HBMS3:
      this->process_hbms3_line_(parts);
      break;
    case ExpectedResponse::HEEP3:
      this->process_heep3_line_(parts);
      break;
    case ExpectedResponse::HSTS2:
      this->process_hsts2_line_(parts);
      break;
    case ExpectedResponse::HCTMSG1:
      this->process_hctmsg1_line_(parts);
      break;
    case ExpectedResponse::QPRTL:
      this->process_qprtl_line_(parts);
      break;
  }

  this->set_timeout("command_throttle", this->command_throttle_ms_, [this]() { this->try_send_next_query_(); });
}

void EasunSmtIiiComponent::process_hop_line_(const std::vector<std::string> &parts) {
  if (parts.size() < 9) {
    this->record_dropped_response_("HOP: only " + std::to_string(parts.size()) + " fields (typ. 9), dropping line");
    return;
  }
  float output_voltage;
  float output_frequency;
  float output_apparent_power;
  float output_active_power;
  float load_percentage;
  float bus_voltage;
  float rated_power;
  float output_current;
  if (!parse_float_field(parts[0], &output_voltage) || !parse_float_field(parts[1], &output_frequency) ||
      !parse_float_field(parts[2], &output_apparent_power) || !parse_float_field(parts[3], &output_active_power) ||
      !parse_float_field(parts[4], &load_percentage) || !parse_float_field(parts[5], &bus_voltage) ||
      !parse_float_field(parts[6], &rated_power) || !parse_float_field(parts[7], &output_current)) {
    this->record_dropped_response_("HOP: non-numeric field in numeric payload, dropping line");
    return;
  }
  if (output_voltage < 0.0f || output_voltage > 300.0f || output_frequency < 0.0f ||
      (output_frequency > 0.0f && (output_frequency < 40.0f || output_frequency > 70.0f)) ||
      output_apparent_power < 0.0f || output_active_power < 0.0f || load_percentage < 0.0f ||
      load_percentage > 250.0f || bus_voltage < 0.0f || bus_voltage > 600.0f || rated_power < 100.0f ||
      rated_power > 50000.0f || output_current < 0.0f || output_apparent_power > rated_power * 2.0f ||
      output_active_power > rated_power * 2.0f) {
    char reason[160];
    snprintf(reason, sizeof(reason),
             "HOP: implausible values (U=%.1fV f=%.1fHz P=%.0fW S=%.0fVA load=%.0f%% bus=%.1fV rated=%.0f), "
             "dropping line",
             output_voltage, output_frequency, output_active_power, output_apparent_power, load_percentage, bus_voltage,
             rated_power);
    this->record_dropped_response_(reason);
    return;
  }
  this->publish_numeric_(ExpectedResponse::HOP, 0, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 1, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 2, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 3, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 4, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 5, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 6, parts);
  this->publish_numeric_(ExpectedResponse::HOP, 7, parts);
  this->publish_text_(ExpectedResponse::HOP, 8, parts);
}

void EasunSmtIiiComponent::process_hgrid_line_(const std::vector<std::string> &parts) {
  if (parts.size() < 10) {
    this->record_dropped_response_("HGRID: only " + std::to_string(parts.size()) + " fields (typ. 10), dropping line");
    return;
  }
  for (uint8_t i = 0; i <= 8; i++) {
    this->publish_numeric_(ExpectedResponse::HGRID, i, parts);
  }
  this->publish_text_(ExpectedResponse::HGRID, 9, parts);
}

void EasunSmtIiiComponent::process_hbat_line_(const std::vector<std::string> &parts) {
  if (parts.size() < 8) {
    this->record_dropped_response_("HBAT: only " + std::to_string(parts.size()) + " fields (typ. 8), dropping line");
    return;
  }
  for (uint8_t i = 0; i <= 5; i++) {
    this->publish_numeric_(ExpectedResponse::HBAT, i, parts);
  }
  this->publish_text_(ExpectedResponse::HBAT, 6, parts);
  this->publish_text_(ExpectedResponse::HBAT, 7, parts);
}

void EasunSmtIiiComponent::process_hsts_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 3; i++) {
    this->publish_text_(ExpectedResponse::HSTS, i, parts);
  }
}

void EasunSmtIiiComponent::process_hpv_line_(const std::vector<std::string> &parts) {
  this->publish_numeric_(ExpectedResponse::HPV, 0, parts);
  this->publish_numeric_(ExpectedResponse::HPV, 1, parts);
  this->publish_numeric_(ExpectedResponse::HPV, 2, parts);
  this->publish_text_(ExpectedResponse::HPV, 3, parts);
  this->publish_text_(ExpectedResponse::HPV, 4, parts);
  this->publish_text_(ExpectedResponse::HPV, 5, parts);
  this->publish_numeric_(ExpectedResponse::HPV, 6, parts);
  this->publish_numeric_(ExpectedResponse::HPV, 7, parts);
  this->publish_numeric_(ExpectedResponse::HPV, 8, parts);
}

void EasunSmtIiiComponent::process_htemp_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 6; i++) {
    this->publish_numeric_(ExpectedResponse::HTEMP, i, parts);
  }
  this->publish_text_(ExpectedResponse::HTEMP, 7, parts);
}

void EasunSmtIiiComponent::process_heep1_line_(const std::vector<std::string> &parts) {
  if (this->output_source_priority_select_ != nullptr && parts.size() > 0) {
    const auto &field = parts[0];
    const auto &options = OutputSourcePrioritySelect::allowed_options();
    bool published = false;

    // Primarily a numeric code 0..3.
    if (!field.empty() && std::isdigit(static_cast<unsigned char>(field[0]))) {
      const auto code = static_cast<uint8_t>(strtoul(field.c_str(), nullptr, 10));
      if (code < options.size()) {
        this->output_source_priority_select_->publish_state(options[code]);
        published = true;
      }
    }

    // Some firmwares may return a text code (SUB/SBU/SUF/PEC) or a label.
    if (!published) {
      for (size_t i = 0; i < options.size(); i++) {
        if (option_matches_code3_(field, options[i])) {
          this->output_source_priority_select_->publish_state(options[i]);
          break;
        }
      }
    }
  }
  publish_number_state(this->max_charging_current_number_, parts, 1, 0);
  if (this->maximum_mains_charging_current_select_ != nullptr && parts.size() > 2) {
    const auto amps = static_cast<uint16_t>(strtoul(parts[2].c_str(), nullptr, 10));
    const auto &values = MuchgcCurrentSelect::allowed_values();
    const auto &options = MuchgcCurrentSelect::allowed_options();
    for (size_t i = 0; i < values.size(); i++) {
      if (values[i] == amps) {
        this->maximum_mains_charging_current_select_->publish_state(options[i]);
        break;
      }
    }
  }
  // HEEP1 field 3 is a composite string (e.g. "14201110230"), not a number.
  // Mapping of positions (0-based) to parameters:
  //   [0] = Grid Working Range (PGR)                       - 0=APL, 1=UPS
  //   [1] = Grid Connection Protocol Type Setting (RS)     - 1..5
  //   [2] = Battery Type Setting (PBT)                     - 0..9
  //   [4] = Overload Auto Restart (P[DE]u)                 - 0=Disable, 1=Enable
  //   [5] = Input Source Detection Prompt Sound (P[DE]y)   - 0=Disable, 1=Enable
  //   [6] = Over Temp Auto Restart (P[DE]v)                - 0=Disable, 1=Enable
  //   [8..10] = Rated Voltage Setting (V)                  - 220/230/240 V
  //   [3], [7] not yet identified.
  if (parts.size() > 3) {
    const std::string &composite = parts[3];
    if (this->grid_working_range_select_ != nullptr && composite.size() >= 1) {
      const char c = composite[0];
      if (c >= '0' && c <= '1') {
        const auto &options = GridWorkingRangeSelect::allowed_options();
        this->grid_working_range_select_->publish_state(options[c - '0']);
      }
    }
    if (this->grid_connection_protocol_type_select_ != nullptr && composite.size() >= 2) {
      const char c = composite[1];
      if (c >= '1' && c <= '5') {
        const auto &options = GridConnectionProtocolTypeSelect::allowed_options();
        this->grid_connection_protocol_type_select_->publish_state(options[c - '1']);
      }
    }
    if (this->battery_type_select_ != nullptr && composite.size() >= 3) {
      const char c = composite[2];
      if (c >= '0' && c <= '9') {
        const auto &options = BatteryTypeSelect::allowed_options();
        this->battery_type_select_->publish_state(options[c - '0']);
      }
    }
    this->publish_enable_disable_(this->overload_auto_restart_select_, composite, 4);
    this->publish_enable_disable_(this->input_source_detection_prompt_sound_select_, composite, 5);
    this->publish_enable_disable_(this->over_temp_auto_restart_select_, composite, 6);
    if (this->rated_voltage_select_ != nullptr && composite.size() >= 11) {
      const uint16_t volts = static_cast<uint16_t>(strtoul(composite.substr(8, 3).c_str(), nullptr, 10));
      const auto &values = RatedVoltageSelect::allowed_values();
      const auto &options = RatedVoltageSelect::allowed_options();
      for (size_t i = 0; i < values.size(); i++) {
        if (values[i] == volts) {
          this->rated_voltage_select_->publish_state(options[i]);
          break;
        }
      }
    }
  }
  // HEEP1 field 4 is also composite (e.g. "101"):
  //   [0] = Rated Frequency                        - 0=50 Hz, 1=60 Hz
  //   [1] = Display Auto Return Homepage (P[DE]k)  - 0=Disable, 1=Enable
  //   [2] = Charger Priority (PCP)                 - 0..3 (CSO/SNU/OSO/SOR)
  if (parts.size() > 4) {
    const std::string &composite = parts[4];
    if (this->rated_frequency_select_ != nullptr && composite.size() >= 1) {
      const char c = composite[0];
      if (c >= '0' && c <= '1') {
        const auto &options = RatedFrequencySelect::allowed_options();
        this->rated_frequency_select_->publish_state(options[c - '0']);
      }
    }
    this->publish_enable_disable_(this->display_auto_return_homepage_select_, composite, 1);
    if (this->charger_priority_select_ != nullptr && composite.size() >= 3) {
      const char c = composite[2];
      if (c >= '0' && c <= '3') {
        const auto &options = ChargerPrioritySelect::allowed_options();
        this->charger_priority_select_->publish_state(options[c - '0']);
      }
    }
  }
  // HEEP1 field 5 = Buzzer On (P[DE]a) - whole field "0"/"1".
  // HEEP1 field 6 = Backlight On (P[DE]x) - whole field "0"/"1".
  if (parts.size() > 5) {
    this->publish_enable_disable_(this->buzzer_on_select_, parts[5], 0);
  }
  if (parts.size() > 6) {
    this->publish_enable_disable_(this->backlight_on_select_, parts[6], 0);
  }
  this->publish_text_(ExpectedResponse::HEEP1, 7, parts);
  this->publish_text_(ExpectedResponse::HEEP1, 8, parts);
  if (this->bms_function_enable_setting_select_ != nullptr && parts.size() > 9) {
    const auto &options = OnOffCommandSelect::allowed_options();
    const char c = parts[9].empty() ? '\0' : parts[9][0];
    if (c == '0' || c == '1') {
      this->bms_function_enable_setting_select_->publish_state(options[c - '0']);
    }
  }
  publish_number_state(this->bms_lock_machine_battery_capacity_number_, parts, 10, 0);
  publish_number_state(this->restore_mains_charging_battery_capacity_number_, parts, 11, 0);
  publish_number_state(this->restore_battery_discharging_battery_capacity_number_, parts, 12, 0);
  publish_number_state(this->inverter_startup_battery_capacity_number_, parts, 13, 0);
  publish_number_state(this->battery_constant_charging_voltage_setting_number_, parts, 14, 1);
  publish_number_state(this->battery_float_charging_voltage_setting_number_, parts, 15, 1);
  publish_number_state(this->battery_low_voltage_protection_number_, parts, 16, 1);
  this->publish_text_(ExpectedResponse::HEEP1, 17, parts);
  this->publish_text_(ExpectedResponse::HEEP1, 18, parts);
}

void EasunSmtIiiComponent::process_hbms1_line_(const std::vector<std::string> &parts) {
  this->publish_numeric_(ExpectedResponse::HBMS1, 0, parts);
  this->publish_text_(ExpectedResponse::HBMS1, 1, parts);
  for (uint8_t i = 2; i <= 9; i++) {
    this->publish_numeric_(ExpectedResponse::HBMS1, i, parts);
  }
}

void EasunSmtIiiComponent::process_hgen_line_(const std::vector<std::string> &parts) {
  // A valid HGEN response starts with a date (YYMMDD) followed by a time (HH:MM).
  // If the structure does not match, the line is a desynchronized or merged
  // response from another command. Publishing it would push foreign values into
  // the PV energy counters (e.g. the date 260617, or CT constants like 2048 from
  // HEEP3), so drop it instead.
  if (parts.size() < 6 || !field_has_leading_digits(parts[0], 6) ||
      parts[1].find(':') == std::string::npos) {
    this->record_dropped_response_("HGEN: unexpected format (" + std::to_string(parts.size()) +
                                   " fields), dropping line");
    return;
  }
  if (this->system_time_text_sensor_ != nullptr && parts.size() > 1) {
    const std::string &date = parts[0];
    const std::string &time = parts[1];
    if (date.size() >= 6 &&
        isdigit(static_cast<unsigned char>(date[0])) &&
        isdigit(static_cast<unsigned char>(date[1])) &&
        isdigit(static_cast<unsigned char>(date[2])) &&
        isdigit(static_cast<unsigned char>(date[3])) &&
        isdigit(static_cast<unsigned char>(date[4])) &&
        isdigit(static_cast<unsigned char>(date[5])) &&
        time.size() >= 5) {
      char formatted[20];
      snprintf(
          formatted, sizeof(formatted), "20%c%c-%c%c-%c%c %.*s",
          date[0], date[1], date[2], date[3], date[4], date[5], 5, time.c_str());
      this->system_time_text_sensor_->publish_state(formatted);
    } else {
      this->system_time_text_sensor_->publish_state(date + " " + time);
    }
  }
  this->publish_numeric_(ExpectedResponse::HGEN, 2, parts, 1.0f, false);
  this->publish_numeric_(ExpectedResponse::HGEN, 3, parts, 1.0f, false);
  this->publish_numeric_(ExpectedResponse::HGEN, 4, parts, 1.0f, false);
  this->publish_numeric_(ExpectedResponse::HGEN, 5, parts, 1.0f, false);
  this->publish_text_(ExpectedResponse::HGEN, 6, parts);
}

void EasunSmtIiiComponent::process_hpvb_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 5; i++) {
    this->publish_text_(ExpectedResponse::HPVB, i, parts);
  }
}

void EasunSmtIiiComponent::process_heep2_line_(const std::vector<std::string> &parts) {
  if (this->dual_remote_switch_select_ != nullptr && parts.size() > 0) {
    const auto &options = OnOffCommandSelect::allowed_options();
    const char c = parts[0].empty() ? '\0' : parts[0][0];
    if (c == '0' || c == '1') {
      this->dual_remote_switch_select_->publish_state(options[c - '0']);
    }
  }
  this->publish_numeric_(ExpectedResponse::HEEP2, 1, parts);
  this->publish_numeric_(ExpectedResponse::HEEP2, 2, parts);
  publish_number_state(this->low_battery_alarm_voltage_setting_number_, parts, 3, 1);
  publish_number_state(this->battery_recharge_voltage_setting_number_, parts, 4, 1);
  publish_number_state(this->battery_redischarge_voltage_setting_number_, parts, 5, 1);
  if (this->battery_equalization_mode_enable_setting_select_ != nullptr && parts.size() > 6) {
    const auto &options = OnOffCommandSelect::allowed_options();
    const char c = parts[6].empty() ? '\0' : parts[6][0];
    if (c == '0' || c == '1') {
      this->battery_equalization_mode_enable_setting_select_->publish_state(options[c - '0']);
    }
  }
  publish_number_state(this->battery_equalization_voltage_setting_number_, parts, 7, 2);
  publish_number_state(this->battery_equalization_time_number_, parts, 8, 0);
  publish_number_state(this->battery_equalization_timeout_number_, parts, 9, 0);
  publish_number_state(this->battery_equalization_interval_number_, parts, 10, 0);
  if (parts.size() > 11) {
    const std::string &field = parts[11];
    if (field.size() >= 4 && isdigit(field[0]) && isdigit(field[1]) && isdigit(field[2]) && isdigit(field[3])) {
      const uint8_t start_hour = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
      const uint8_t end_hour = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
      if (start_hour <= 23 && end_hour <= 23) {
        this->ac_charging_time_start_hour_ = start_hour;
        this->ac_charging_time_stop_hour_ = end_hour;
        this->ac_charging_time_start_known_ = true;
        this->ac_charging_time_stop_known_ = true;
        char option[8];
        if (this->ac_charging_time_start_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", start_hour);
          this->ac_charging_time_start_select_->publish_state(option);
        }
        if (this->ac_charging_time_stop_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", end_hour);
          this->ac_charging_time_stop_select_->publish_state(option);
        }
      }
    }
  }
  if (parts.size() > 12) {
    const std::string &field = parts[12];
    if (field.size() >= 4 &&
        isdigit(static_cast<unsigned char>(field[0])) &&
        isdigit(static_cast<unsigned char>(field[1])) &&
        isdigit(static_cast<unsigned char>(field[2])) &&
        isdigit(static_cast<unsigned char>(field[3]))) {
      const uint8_t start_hour = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
      const uint8_t end_hour = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
      if (start_hour <= 23 && end_hour <= 23) {
        this->ac_output_time_start_hour_ = start_hour;
        this->ac_output_time_stop_hour_ = end_hour;
        this->ac_output_time_start_known_ = true;
        this->ac_output_time_stop_known_ = true;
        char option[8];
        if (this->ac_output_time_start_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", start_hour);
          this->ac_output_time_start_select_->publish_state(option);
        }
        if (this->ac_output_time_stop_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", end_hour);
          this->ac_output_time_stop_select_->publish_state(option);
        }
      }
    }
  }
  this->publish_numeric_(ExpectedResponse::HEEP2, 13, parts);
  if (parts.size() > 14) {
    const std::string &field = parts[14];
    if (field.size() >= 4 &&
        isdigit(static_cast<unsigned char>(field[0])) &&
        isdigit(static_cast<unsigned char>(field[1])) &&
        isdigit(static_cast<unsigned char>(field[2])) &&
        isdigit(static_cast<unsigned char>(field[3]))) {
      const uint8_t start_hour = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
      const uint8_t stop_hour = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
      if (start_hour <= 23 && stop_hour <= 23) {
        this->output_time_start_hour_ = start_hour;
        this->output_time_stop_hour_ = stop_hour;
        this->output_time_start_known_ = true;
        this->output_time_stop_known_ = true;
        char option[8];
        if (this->output_time_start_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", start_hour);
          this->output_time_start_select_->publish_state(option);
        }
        if (this->output_time_stop_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", stop_hour);
          this->output_time_stop_select_->publish_state(option);
        }
      }
    }
  }
  this->publish_numeric_(ExpectedResponse::HEEP2, 15, parts);
  if (parts.size() > 16) {
    const std::string &field = parts[16];
    if (field.size() >= 5 &&
        isdigit(static_cast<unsigned char>(field[0])) &&
        isdigit(static_cast<unsigned char>(field[1])) &&
        isdigit(static_cast<unsigned char>(field[2])) &&
        isdigit(static_cast<unsigned char>(field[3])) &&
        isdigit(static_cast<unsigned char>(field[4]))) {
      const auto pct = static_cast<float>((field[0] - '0') * 10 + (field[1] - '0'));
      const auto mins = static_cast<float>((field[2] - '0') * 100 + (field[3] - '0') * 10 + (field[4] - '0'));
      if (this->restore_second_output_battery_capacity_number_ != nullptr) {
        this->restore_second_output_battery_capacity_number_->publish_state(pct);
      }
      if (this->second_output_discharge_time_number_ != nullptr) {
        this->second_output_discharge_time_number_->publish_state(mins);
      }
    }
  }
}

void EasunSmtIiiComponent::process_himsg1_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 2; i++) {
    this->publish_text_(ExpectedResponse::HIMSG1, i, parts);
  }
}

void EasunSmtIiiComponent::process_hbms2_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 8; i++) {
    this->publish_text_(ExpectedResponse::HBMS2, i, parts);
  }
}

void EasunSmtIiiComponent::process_hbms3_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 16; i++) {
    this->publish_text_(ExpectedResponse::HBMS3, i, parts);
  }
}

void EasunSmtIiiComponent::process_heep3_line_(const std::vector<std::string> &parts) {
  this->publish_text_(ExpectedResponse::HEEP3, 0, parts);
  publish_number_state(this->discharge_current_limit_number_, parts, 6, 0);
  this->publish_text_(ExpectedResponse::HEEP3, 1, parts);
  this->publish_text_(ExpectedResponse::HEEP3, 2, parts);
  publish_number_state(this->ct_zero_power_number_, parts, 3, 0);
  this->publish_numeric_(ExpectedResponse::HEEP3, 4, parts);
  publish_number_state(this->grid_connected_power_number_, parts, 5, 0);
  this->publish_text_(ExpectedResponse::HEEP3, 7, parts);
  this->publish_text_(ExpectedResponse::HEEP3, 8, parts);
  this->publish_text_(ExpectedResponse::HEEP3, 9, parts);
  this->publish_text_(ExpectedResponse::HEEP3, 10, parts);
  if (parts.size() > 11) {
    const std::string &field = parts[11];
    if (field.size() >= 4 && isdigit(field[0]) && isdigit(field[1]) && isdigit(field[2]) && isdigit(field[3])) {
      const uint8_t start_hour = static_cast<uint8_t>((field[0] - '0') * 10 + (field[1] - '0'));
      const uint8_t end_hour = static_cast<uint8_t>((field[2] - '0') * 10 + (field[3] - '0'));
      if (start_hour <= 23 && end_hour <= 23) {
        this->battery_energy_grid_feeding_time_start_hour_ = start_hour;
        this->battery_energy_grid_feeding_time_stop_hour_ = end_hour;
        this->battery_energy_grid_feeding_time_start_known_ = true;
        this->battery_energy_grid_feeding_time_stop_known_ = true;
        char option[8];
        if (this->battery_energy_grid_feeding_time_start_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", start_hour);
          this->battery_energy_grid_feeding_time_start_select_->publish_state(option);
        }
        if (this->battery_energy_grid_feeding_time_stop_select_ != nullptr) {
          snprintf(option, sizeof(option), "%02u:00", end_hour);
          this->battery_energy_grid_feeding_time_stop_select_->publish_state(option);
        }
      }
    }
  }
  this->publish_text_(ExpectedResponse::HEEP3, 12, parts);
}

void EasunSmtIiiComponent::process_hsts2_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 3; i++) {
    this->publish_text_(ExpectedResponse::HSTS2, i, parts);
  }
}

void EasunSmtIiiComponent::process_hctmsg1_line_(const std::vector<std::string> &parts) {
  for (uint8_t i = 0; i <= 9; i++) {
    this->publish_text_(ExpectedResponse::HCTMSG1, i, parts);
  }
}

void EasunSmtIiiComponent::process_qprtl_line_(const std::vector<std::string> &parts) {
  this->publish_text_(ExpectedResponse::QPRTL, 0, parts);
}

}  // namespace easun_smt_iii
}  // namespace esphome
