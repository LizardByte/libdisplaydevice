/**
 * @file src/macos/include/display_device/macos/mac_api_layer_interface.h
 * @brief Declarations for the MacApiLayerInterface.
 */
#pragma once

// local includes
#include "types.h"

namespace display_device {
  /**
   * @brief Lowest level macOS API wrapper for easy mocking.
   */
  class MacApiLayerInterface {
  public:
    /**
     * @brief Default virtual destructor.
     */
    virtual ~MacApiLayerInterface() = default;

    /**
     * @brief Check if display configuration APIs are accessible.
     * @returns True if macOS display APIs can be called, false otherwise.
     */
    [[nodiscard]] virtual bool isApiAccessAvailable() const = 0;

    /**
     * @brief Stringify a macOS display API error code.
     * @param error_code Error code to stringify.
     * @returns String containing a readable error description.
     */
    [[nodiscard]] virtual std::string getErrorString(MacApiError error_code) const = 0;

    /**
     * @brief Query macOS for display identifiers.
     * @param type Display list type to query.
     * @returns Display identifiers matching the query.
     */
    [[nodiscard]] virtual MacDisplayIdList getDisplayIds(MacQueryType type) const = 0;

    /**
     * @brief Tell macOS that the user is active and displays should wake.
     * @param reason Short human-readable reason for the wake assertion.
     * @returns Power assertion id to release later, or empty optional on failure.
     */
    [[nodiscard]] virtual std::optional<MacPowerAssertionId> declareUserActivity(const std::string &reason) = 0;

    /**
     * @brief Create a power assertion that prevents user-idle display sleep.
     * @param reason Short human-readable reason for the assertion.
     * @returns Power assertion id to release later, or empty optional on failure.
     */
    [[nodiscard]] virtual std::optional<MacPowerAssertionId> createDisplaySleepAssertion(const std::string &reason) = 0;

    /**
     * @brief Release a macOS power assertion.
     * @param assertion_id Assertion id returned by declareUserActivity or createDisplaySleepAssertion.
     * @returns True if the assertion was released, false otherwise.
     */
    [[nodiscard]] virtual bool releasePowerAssertion(MacPowerAssertionId assertion_id) = 0;

    /**
     * @brief Get the library device id for a display.
     * @param display_id Display to query.
     * @returns Stable best-effort device id, or empty string if unavailable.
     */
    [[nodiscard]] virtual std::string getDeviceId(MacDisplayId display_id) const = 0;

    /**
     * @brief Get the current display mode.
     * @param display_id Display to query.
     * @returns Current display mode, or empty optional if unavailable.
     */
    [[nodiscard]] virtual std::optional<MacDisplayMode> getCurrentDisplayMode(MacDisplayId display_id) const = 0;

    /**
     * @brief Get available display modes for a display.
     * @param display_id Display to query.
     * @returns Available display modes.
     */
    [[nodiscard]] virtual MacDisplayModeList getDisplayModes(MacDisplayId display_id) const = 0;

    /**
     * @brief Get the macOS capture selector for a display.
     * @param display_id Display to query.
     * @returns Decimal CoreGraphics display id string, or empty string if unavailable.
     */
    [[nodiscard]] virtual std::string getDisplayName(MacDisplayId display_id) const = 0;

    /**
     * @brief Get a human-readable display name.
     * @param display_id Display to query.
     * @returns Friendly display name or empty string if unavailable.
     */
    [[nodiscard]] virtual std::string getFriendlyName(MacDisplayId display_id) const = 0;

    /**
     * @brief Get EDID byte array for a display.
     * @param display_id Display to query.
     * @returns EDID byte array, or an empty array if unavailable.
     */
    [[nodiscard]] virtual std::vector<std::byte> getEdid(MacDisplayId display_id) const = 0;

    /**
     * @brief Get the display scale value.
     * @param display_id Display to query.
     * @returns Display scale, or empty optional if unavailable.
     */
    [[nodiscard]] virtual std::optional<Rational> getDisplayScale(MacDisplayId display_id) const = 0;

    /**
     * @brief Get the display origin point.
     * @param display_id Display to query.
     * @returns Display origin, or empty optional if unavailable.
     */
    [[nodiscard]] virtual std::optional<Point> getOriginPoint(MacDisplayId display_id) const = 0;

    /**
     * @brief Check whether a display is the main display.
     * @param display_id Display to check.
     * @returns True if the display is main, false otherwise.
     */
    [[nodiscard]] virtual bool isMainDisplay(MacDisplayId display_id) const = 0;

    /**
     * @brief Check whether a display is active.
     * @param display_id Display to check.
     * @returns True if the display is active, false otherwise.
     */
    [[nodiscard]] virtual bool isActive(MacDisplayId display_id) const = 0;

    /**
     * @brief Check whether a display is online.
     * @param display_id Display to check.
     * @returns True if the display is online, false otherwise.
     */
    [[nodiscard]] virtual bool isOnline(MacDisplayId display_id) const = 0;

    /**
     * @brief Get the display mirrored by the specified display.
     * @param display_id Display to query.
     * @returns Master display id, or zero if the display is not a secondary mirror.
     */
    [[nodiscard]] virtual MacDisplayId getMirrorMaster(MacDisplayId display_id) const = 0;

    /**
     * @brief Set the display mode for a display.
     * @param display_id Display to modify.
     * @param mode Mode to apply.
     * @returns True if the display mode was applied, false otherwise.
     */
    [[nodiscard]] virtual bool setDisplayMode(MacDisplayId display_id, const MacDisplayMode &mode) = 0;

    /**
     * @brief Set the origin point for a display.
     * @param display_id Display to modify.
     * @param origin Origin point to apply.
     * @returns True if the origin was applied, false otherwise.
     */
    [[nodiscard]] virtual bool setOriginPoint(MacDisplayId display_id, const Point &origin) = 0;

    /**
     * @brief Set a display as a mirror of another display.
     * @param display_id Display to modify.
     * @param master_display_id Master display to mirror.
     * @returns True if mirroring was applied, false otherwise.
     */
    [[nodiscard]] virtual bool setMirror(MacDisplayId display_id, MacDisplayId master_display_id) = 0;
  };
}  // namespace display_device
