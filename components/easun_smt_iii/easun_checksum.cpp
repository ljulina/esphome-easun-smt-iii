// EASUN SMT-III - ESPHome external component
//
// CRC helper for the SMT-III serial protocol.
//
// Project: https://github.com/ljulina/esphome-easun-smt-iii
// Author:  Ladislav Julina <ljulina@email.cz>
// License: MIT (see LICENSE)

#include "easun_checksum.h"
#include "esphome/core/log.h"

static const char *const TAG = "easun_smt_iii.checksum";

namespace esphome {
namespace easun_smt_iii {

// CRC-16/XMODEM: poly 0x1021, init 0x0000, refin=false, refout=false, xorout=0x0000
static uint16_t crc16_xmodem(const uint8_t *data, size_t len) {
  uint16_t crc = 0x0000;
  for (size_t i = 0; i < len; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (uint8_t b = 0; b < 8; b++) {
      if (crc & 0x8000) {
        crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
      } else {
        crc <<= 1;
      }
    }
  }
  return crc;
}

uint16_t checksum_encode(const uint8_t *payload, size_t len) {
  uint16_t c = crc16_xmodem(payload, len);
  ESP_LOGD(TAG, "checksum_encode (xmodem) = 0x%04X for len=%u", c, static_cast<unsigned>(len));
  return c;
}

bool checksum_verify_frame(const uint8_t *frame, size_t len) {
  // Expected form: ... payload ... | crc_hi | crc_lo | 0x0D
  if (len < 4 || frame[len - 1] != '\r')
    return false;
  size_t body = len - 3;
  if (body == 0)
    return false;
  uint16_t got = (static_cast<uint16_t>(frame[body]) << 8) | frame[body + 1];
  uint16_t calc = crc16_xmodem(frame, body);
  if (calc == got) {
    return true;
  }
  ESP_LOGW(TAG, "checksum mismatch: got 0x%04X xmodem_calc 0x%04X", got, calc);
  return false;
}

}  // namespace easun_smt_iii
}  // namespace esphome
