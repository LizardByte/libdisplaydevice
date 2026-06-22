/**
 * @file src/common/include/display_device/detail/persistent_state_utils.h
 * @brief Shared helpers for persistent state wrappers.
 */
#pragma once

// system includes
#include <cstdint>
#include <iterator>
#include <optional>
#include <string_view>
#include <vector>

// local includes
#include "display_device/logging.h"
#include "display_device/settings_persistence_interface.h"

namespace display_device::detail {
  /**
   * @brief Persist state and update the cached copy after a successful write.
   * @tparam State Cached state type.
   * @tparam SerializeFn Callable type used to serialize the state.
   * @param settings_persistence_api Persistence API used to store or clear state.
   * @param cached_state Cached state to compare and update.
   * @param state New state to persist.
   * @param serialize_state Callable that serializes a state and updates a success flag.
   * @param serialize_error_message Error message used when serialization fails.
   * @return True if the state was already current or was persisted successfully, false otherwise.
   */
  template<typename State, typename SerializeFn>
  [[nodiscard]] bool persistState(
    SettingsPersistenceInterface &settings_persistence_api,
    std::optional<State> &cached_state,
    const std::optional<State> &state,
    const SerializeFn &serialize_state,
    const std::string_view serialize_error_message
  ) {
    if (cached_state == state) {
      return true;
    }

    if (!state) {
      if (!settings_persistence_api.clear()) {
        return false;
      }

      cached_state = std::nullopt;
      return true;
    }

    bool success {false};
    const auto serialized_state {serialize_state(*state, success)};
    if (!success) {
      DD_LOG(error) << serialize_error_message << "\n"
                    << serialized_state;
      return false;
    }

    if (!settings_persistence_api.store({std::begin(serialized_state), std::end(serialized_state)})) {
      return false;
    }

    cached_state = *state;
    return true;
  }
}  // namespace display_device::detail
