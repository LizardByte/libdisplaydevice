// local includes
#include "mockwindisplaydevice.h"
#include "helpers.h"

namespace ut_consts {
  const std::optional<display_device::SingleDisplayConfigState> SDCS_NULL { std::nullopt };
  const std::optional<display_device::SingleDisplayConfigState> SDCS_EMPTY { display_device::SingleDisplayConfigState {} };
  const std::optional<display_device::SingleDisplayConfigState> SDCS_FULL { []() {
    const display_device::SingleDisplayConfigState state {
      { { { "DeviceId1" } },
        { "DeviceId1" } },
      { display_device::SingleDisplayConfigState::Modified {
        { { "DeviceId2" }, { "DeviceId3" } },
        { { "DeviceId2", { { 1920, 1080 }, { 120, 1 } } },
          { "DeviceId3", { { 1920, 1080 }, { 60, 1 } } } },
        { { "DeviceId2", { display_device::HdrState::Disabled } },
          { "DeviceId3", std::nullopt } },
        { "DeviceId3" },
      } }
    };

    return state;
  }() };
  const std::optional<display_device::SingleDisplayConfigState> SDCS_NO_MODIFICATIONS { []() {
    const display_device::SingleDisplayConfigState state {
      { { { "DeviceId1" } },
        { "DeviceId1" } },
      { display_device::SingleDisplayConfigState::Modified {
        { { "DeviceId2" }, { "DeviceId3" } } } }
    };

    return state;
  }() };
}  // namespace ut_consts
