#pragma once

// EASUN SMT-III - ESPHome external component
//
// CRC helper for the SMT-III serial protocol.
//
// Project: https://github.com/ljulina/esphome-easun-smt-iii
// Author:  Ladislav Julina <ljulina@email.cz>
// License: MIT (see LICENSE)

#include <cstddef>
#include <cstdint>

namespace esphome {
namespace easun_smt_iii {

/// CRC for SMT-III commands.
/// Verified on samples `MUCHGC000`, `^S???RS01`, `PBT00`, etc.:
/// CRC-16/XMODEM, poly 0x1021, init 0x0000, refin=false, refout=false, xorout=0x0000.
/// On the wire it is sent as high-byte, low-byte, then `0x0D`.
uint16_t checksum_encode(const uint8_t *payload, size_t len);

bool checksum_verify_frame(const uint8_t *frame, size_t len);

}  // namespace easun_smt_iii
}  // namespace esphome
