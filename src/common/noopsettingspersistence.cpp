// local includes
#include "displaydevice/noopsettingspersistence.h"

namespace display_device {
  bool
  NoopSettingsPersistence::store(const std::vector<std::uint8_t> &) {
    return true;
  }

  std::optional<std::vector<std::uint8_t>>
  NoopSettingsPersistence::load() const {
    return std::vector<std::uint8_t> {};
  }

  void
  NoopSettingsPersistence::clear() {
  }
}  // namespace display_device
