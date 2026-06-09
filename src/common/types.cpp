/**
 * @file src/common/types.cpp
 * @brief Definitions for common display device types.
 */
// header include
#include "display_device/types.h"

// system includes
#include <array>
#include <iomanip>
#include <sstream>

// local includes
#include "display_device/logging.h"

namespace {
  std::byte operator+(const std::byte lhs, const std::byte &rhs) {
    return std::byte {static_cast<std::uint8_t>(static_cast<int>(lhs) + static_cast<int>(rhs))};
  }

  // This madness should be removed once the minimum compiler version increases...
  std::byte logicalAnd(const std::byte &lhs, const std::byte &rhs) {
    return std::byte {static_cast<std::uint8_t>(static_cast<int>(lhs) & static_cast<int>(rhs))};
  }

  // This madness should be removed once the minimum compiler version increases...
  std::byte shiftLeft(const std::byte &lhs, const int rhs) {
    return std::byte {static_cast<std::uint8_t>(static_cast<int>(lhs) << rhs)};
  }

  // This madness should be removed once the minimum compiler version increases...
  std::byte shiftRight(const std::byte &lhs, const int rhs) {
    return std::byte {static_cast<std::uint8_t>(static_cast<int>(lhs) >> rhs)};
  }
}  // namespace

namespace display_device {
  std::optional<EdidData> EdidData::parse(const std::vector<std::byte> &data) {
    if (data.empty()) {
      return std::nullopt;
    }

    if (data.size() < 128) {
      DD_LOG(warning) << "EDID data size is too small: " << data.size();
      return std::nullopt;
    }

    // ---- Verify fixed header
    if (static const std::array fixed_header {std::byte {0x00}, std::byte {0xFF}, std::byte {0xFF}, std::byte {0xFF}, std::byte {0xFF}, std::byte {0xFF}, std::byte {0xFF}, std::byte {0x00}};
        !std::equal(std::begin(fixed_header), std::end(fixed_header), std::begin(data))) {
      DD_LOG(warning) << "EDID data does not contain fixed header.";
      return std::nullopt;
    }

    // ---- Verify checksum
    {
      int sum = 0;
      for (std::size_t i = 0; i < 128; ++i) {
        sum += static_cast<int>(data[i]);
      }

      if (sum % 256 != 0) {
        DD_LOG(warning) << "EDID checksum verification failed.";
        return std::nullopt;
      }
    }

    EdidData edid {};

    // ---- Get manufacturer ID (ASCII code A-Z)
    {
      constexpr std::byte ascii_offset {'@'};

      const auto byte_a {data[8]};
      const auto byte_b {data[9]};
      std::array<char, 3> man_id {};

      man_id[0] = static_cast<char>(ascii_offset + shiftRight(logicalAnd(byte_a, std::byte {0x7C}), 2));
      man_id[1] = static_cast<char>(ascii_offset + shiftLeft(logicalAnd(byte_a, std::byte {0x03}), 3) + shiftRight(logicalAnd(byte_b, std::byte {0xE0}), 5));
      man_id[2] = static_cast<char>(ascii_offset + logicalAnd(byte_b, std::byte {0x1F}));

      for (const char ch : man_id) {
        if (ch < 'A' || ch > 'Z') {
          DD_LOG(warning) << "EDID manufacturer id is out of range.";
          return std::nullopt;
        }
      }

      edid.m_manufacturer_id = {std::begin(man_id), std::end(man_id)};
    }

    // ---- Product code (HEX representation)
    {
      std::uint16_t prod_num {0};
      prod_num |= static_cast<int>(data[10]) << 0;
      prod_num |= static_cast<int>(data[11]) << 8;

      std::stringstream stream;
      stream << std::setfill('0') << std::setw(4) << std::hex << std::uppercase << prod_num;
      edid.m_product_code = stream.str();
    }

    // ---- Serial number
    {
      std::uint32_t serial_num {0};
      serial_num |= static_cast<int>(data[12]) << 0;
      serial_num |= static_cast<int>(data[13]) << 8;
      serial_num |= static_cast<int>(data[14]) << 16;
      serial_num |= static_cast<int>(data[15]) << 24;

      edid.m_serial_number = serial_num;
    }

    return edid;
  }
}  // namespace display_device
