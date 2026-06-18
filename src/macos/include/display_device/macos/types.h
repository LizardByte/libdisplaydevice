/**
 * @file src/macos/include/display_device/macos/types.h
 * @brief Declarations for macOS specific display device types.
 */
#pragma once

// system includes
#include <functional>
#include <optional>
#include <string>
#include <vector>

// local includes
#include "display_device/types.h"

namespace display_device {
  /**
   * @brief Error code returned by macOS display APIs.
   */
  using MacApiError = int;

  /**
   * @brief CoreGraphics display identifier.
   *
   * CoreGraphics exposes display identifiers as 32-bit values. The platform
   * layer keeps this type independent from CoreGraphics headers so public
   * headers remain easy to parse on non-Apple hosts.
   */
  using MacDisplayId = std::uint32_t;

  /**
   * @brief A list of CoreGraphics display identifiers.
   */
  using MacDisplayIdList = std::vector<MacDisplayId>;

  /**
   * @brief Type of display list to query from macOS.
   */
  enum class MacQueryType {
    Active,  ///< Displays that are drawable.
    Online  ///< Displays that are connected to the system.
  };

  /**
   * @brief A LIST[LIST[DEVICE_ID]] structure which represents active macOS display topology.
   *
   * Each inner list is a mirrored display group. Each top-level entry is an
   * extended display region.
   */
  using MacActiveTopology = std::vector<std::vector<std::string>>;

  /**
   * @brief Display mode data used by the macOS backend.
   */
  struct MacDisplayMode {
    Resolution m_resolution {};  ///< Display resolution in pixels.
    Rational m_refresh_rate {};  ///< Display refresh rate.

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const MacDisplayMode &lhs, const MacDisplayMode &rhs) = default;
  };

  /**
   * @brief A list of macOS display modes.
   */
  using MacDisplayModeList = std::vector<MacDisplayMode>;

  /**
   * @brief Ordered map of [DEVICE_ID -> MacDisplayMode].
   */
  using MacDeviceDisplayModeMap = StringMap<MacDisplayMode>;

  /**
   * @brief Ordered map of [DEVICE_ID -> std::optional<HdrState>].
   */
  using MacHdrStateMap = StringMap<std::optional<HdrState>>;

  /**
   * @brief Arbitrary macOS data for making and undoing settings changes.
   */
  struct MacSingleDisplayConfigState {
    /**
     * @brief Original system state used as a base for revert operations.
     */
    struct Initial {
      MacActiveTopology m_topology {};  ///< Original active topology.
      StringSet m_primary_devices {};  ///< Original primary device IDs.

      /**
       * @brief Comparator for strict equality.
       */
      friend bool operator==(const Initial &lhs, const Initial &rhs) = default;
    };

    /**
     * @brief System state modified by this library.
     */
    struct Modified {
      MacActiveTopology m_topology {};  ///< Modified active topology.
      MacDeviceDisplayModeMap m_original_modes {};  ///< Original display modes before modification.
      MacHdrStateMap m_original_hdr_states {};  ///< Original HDR states before modification.
      std::string m_original_primary_device {};  ///< Original primary device before modification.

      /**
       * @brief Check if the changed topology has any other modifications.
       * @return True if DisplayMode, HDR or primary device has been changed, false otherwise.
       */
      [[nodiscard]] bool hasModifications() const;

      /**
       * @brief Comparator for strict equality.
       */
      friend bool operator==(const Modified &lhs, const Modified &rhs) = default;
    };

    Initial m_initial;  ///< Initial system state.
    Modified m_modified;  ///< Modified system state.

    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const MacSingleDisplayConfigState &lhs, const MacSingleDisplayConfigState &rhs) = default;
  };

  /**
   * @brief Default function type used for cleanup/guard functions.
   */
  using MacDdGuardFn = std::function<void()>;

  /**
   * @brief Settings for macOS-specific workarounds.
   */
  struct MacWorkarounds {
    /**
     * @brief Comparator for strict equality.
     */
    friend bool operator==(const MacWorkarounds &lhs, const MacWorkarounds &rhs) = default;
  };
}  // namespace display_device
