/**
 * @file src/macos/mac_api_layer.cpp
 * @brief Definitions for the MacApiLayer.
 */
// class header include
#include "display_device/macos/mac_api_layer.h"

// system includes
#include <algorithm>
#include <cmath>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <cstdint>
#include <cstring>
#include <IOKit/graphics/IOGraphicsLib.h>
#include <IOKit/graphics/IOGraphicsTypes.h>
#include <IOKit/IOKitLib.h>
#include <iomanip>
#include <limits>
#include <numeric>
#include <sstream>

namespace display_device {
  namespace {
    /**
     * @brief Small RAII wrapper for CoreFoundation objects.
     */
    template<typename T>
    class CfPtr {
    public:
      /**
       * @brief Default constructor.
       */
      CfPtr() = default;

      /**
       * @brief Construct from a CoreFoundation object.
       * @param ref Reference to own.
       */
      explicit CfPtr(T ref):
          m_ref {ref} {}

      CfPtr(const CfPtr &) = delete;
      CfPtr &operator=(const CfPtr &) = delete;

      /**
       * @brief Move constructor.
       * @param other Object to move from.
       */
      CfPtr(CfPtr &&other) noexcept:
          m_ref {other.m_ref} {
        other.m_ref = nullptr;
      }

      /**
       * @brief Move assignment.
       * @param other Object to move from.
       * @return Reference to this object.
       */
      CfPtr &operator=(CfPtr &&other) noexcept {
        if (this != &other) {
          reset(other.m_ref);
          other.m_ref = nullptr;
        }

        return *this;
      }

      /**
       * @brief Destructor.
       */
      ~CfPtr() {
        reset(nullptr);
      }

      /**
       * @brief Get the wrapped reference.
       * @return Wrapped reference.
       */
      [[nodiscard]] T get() const {
        return m_ref;
      }

      /**
       * @brief Reset the wrapped reference.
       * @param ref New reference to own.
       */
      void reset(T ref) {
        if (m_ref) {
          CFRelease(m_ref);
        }

        m_ref = ref;
      }

      /**
       * @brief Check if a reference is wrapped.
       */
      explicit operator bool() const {
        return m_ref != nullptr;
      }

    private:
      T m_ref {nullptr};  ///< Wrapped reference.
    };

    /**
     * @brief Small RAII wrapper for IOKit objects.
     */
    class IoObject {
    public:
      /**
       * @brief Construct from an IOKit object.
       * @param object Object to own.
       */
      explicit IoObject(const io_object_t object):
          m_object {object} {}

      IoObject(const IoObject &) = delete;
      IoObject &operator=(const IoObject &) = delete;

      /**
       * @brief Destructor.
       */
      ~IoObject() {
        if (m_object != IO_OBJECT_NULL) {
          IOObjectRelease(m_object);
        }
      }

    private:
      io_object_t m_object {IO_OBJECT_NULL};  ///< Wrapped object.
    };

    /**
     * @brief Convert a size to unsigned int without overflowing.
     * @param value Value to convert.
     * @return Converted value.
     */
    [[nodiscard]] unsigned int toUnsignedInt(const std::size_t value) {
      return static_cast<unsigned int>(std::min<std::size_t>(value, std::numeric_limits<unsigned int>::max()));
    }

    /**
     * @brief Convert a floating point refresh rate to a rational value.
     * @param value Floating point refresh rate.
     * @return Rational refresh rate.
     */
    [[nodiscard]] Rational toRationalRefreshRate(const double value) {
      if (!std::isfinite(value) || value <= 0.) {
        return {0, 1};
      }

      constexpr std::uint64_t denominator {1000};
      const auto rounded_numerator {static_cast<std::uint64_t>(std::llround(value * static_cast<double>(denominator)))};
      if (rounded_numerator == 0) {
        return {0, 1};
      }

      const auto divisor {std::gcd(rounded_numerator, denominator)};
      return {
        static_cast<unsigned int>(std::min<std::uint64_t>(rounded_numerator / divisor, std::numeric_limits<unsigned int>::max())),
        static_cast<unsigned int>(denominator / divisor)
      };
    }

    /**
     * @brief Convert a CoreGraphics mode to a library mode.
     * @param mode Mode to convert.
     * @return Converted mode, or empty optional if mode data is unusable.
     */
    [[nodiscard]] std::optional<MacDisplayMode> toDisplayMode(CGDisplayModeRef mode) {
      if (!mode) {
        return std::nullopt;
      }

      auto width {CGDisplayModeGetPixelWidth(mode)};
      auto height {CGDisplayModeGetPixelHeight(mode)};
      if (width == 0 || height == 0) {
        width = CGDisplayModeGetWidth(mode);
        height = CGDisplayModeGetHeight(mode);
      }

      if (width == 0 || height == 0) {
        return std::nullopt;
      }

      return MacDisplayMode {
        {toUnsignedInt(width), toUnsignedInt(height)},
        toRationalRefreshRate(CGDisplayModeGetRefreshRate(mode))
      };
    }

    /**
     * @brief Convert a CoreFoundation string to a standard string.
     * @param value String to convert.
     * @return Converted string, or an empty string on failure.
     */
    [[nodiscard]] std::string toString(CFStringRef value) {
      if (!value) {
        return {};
      }

      if (const auto *direct_c_string {CFStringGetCStringPtr(value, kCFStringEncodingUTF8)}) {
        return direct_c_string;
      }

      const auto length {CFStringGetLength(value)};
      const auto max_size {CFStringGetMaximumSizeForEncoding(length, kCFStringEncodingUTF8) + 1};
      std::string result(static_cast<std::size_t>(max_size), '\0');
      if (!CFStringGetCString(value, result.data(), max_size, kCFStringEncodingUTF8)) {
        return {};
      }

      result.resize(std::strlen(result.c_str()));
      return result;
    }

    /**
     * @brief Get a CoreFoundation dictionary value with type validation.
     * @param dictionary Dictionary to query.
     * @param key Key to query.
     * @param expected_type Expected CoreFoundation type id.
     * @return Value when present and type matches.
     */
    [[nodiscard]] CFTypeRef getTypedValue(CFDictionaryRef dictionary, CFStringRef key, const CFTypeID expected_type) {
      if (!dictionary || !key) {
        return nullptr;
      }

      const auto value {CFDictionaryGetValue(dictionary, key)};
      if (!value || CFGetTypeID(value) != expected_type) {
        return nullptr;
      }

      return value;
    }

    /**
     * @brief Get an unsigned integer from a CoreFoundation dictionary.
     * @param dictionary Dictionary to query.
     * @param key Key to query.
     * @return Converted value, or empty optional on failure.
     */
    [[nodiscard]] std::optional<std::uint32_t> getUInt32(CFDictionaryRef dictionary, CFStringRef key) {
      const auto number {static_cast<CFNumberRef>(getTypedValue(dictionary, key, CFNumberGetTypeID()))};
      if (!number) {
        return std::nullopt;
      }

      std::uint32_t result {};
      if (!CFNumberGetValue(number, kCFNumberSInt32Type, &result)) {
        return std::nullopt;
      }

      return result;
    }

    /**
     * @brief Get a string from a CoreFoundation dictionary.
     * @param dictionary Dictionary to query.
     * @param key Key to query.
     * @return Converted value.
     */
    [[nodiscard]] std::string getString(CFDictionaryRef dictionary, CFStringRef key) {
      return toString(static_cast<CFStringRef>(getTypedValue(dictionary, key, CFStringGetTypeID())));
    }

    /**
     * @brief Get the preferred display product name from an IOKit dictionary.
     * @param dictionary Dictionary to query.
     * @return Product name, or an empty string if unavailable.
     */
    [[nodiscard]] std::string getProductName(CFDictionaryRef dictionary) {
      const auto value {CFDictionaryGetValue(dictionary, CFSTR(kDisplayProductName))};
      if (!value) {
        return {};
      }

      if (CFGetTypeID(value) == CFStringGetTypeID()) {
        return toString(static_cast<CFStringRef>(value));
      }

      if (CFGetTypeID(value) != CFDictionaryGetTypeID()) {
        return {};
      }

      const auto names {static_cast<CFDictionaryRef>(value)};
      const auto count {CFDictionaryGetCount(names)};
      std::vector<const void *> values(static_cast<std::size_t>(count), nullptr);
      CFDictionaryGetKeysAndValues(names, nullptr, values.data());

      for (const auto *name : values) {
        if (name && CFGetTypeID(name) == CFStringGetTypeID()) {
          if (auto converted {toString(static_cast<CFStringRef>(name))}; !converted.empty()) {
            return converted;
          }
        }
      }

      return {};
    }

    /**
     * @brief Score how well an IOKit dictionary matches a CoreGraphics display.
     * @param dictionary Dictionary to score.
     * @param display_id Display to match.
     * @return Negative value when incompatible, otherwise match score.
     */
    [[nodiscard]] int getMatchScore(CFDictionaryRef dictionary, const MacDisplayId display_id) {
      int score {0};

      const auto check_number = [&score](const std::optional<std::uint32_t> &value, const std::uint32_t expected, const int weight) {
        if (!value) {
          return true;
        }

        if (expected != 0 && *value != expected) {
          return false;
        }

        score += weight;
        return true;
      };

      if (!check_number(getUInt32(dictionary, CFSTR(kDisplayVendorID)), CGDisplayVendorNumber(display_id), 4)) {
        return -1;
      }

      if (!check_number(getUInt32(dictionary, CFSTR(kDisplayProductID)), CGDisplayModelNumber(display_id), 4)) {
        return -1;
      }

      if (!check_number(getUInt32(dictionary, CFSTR(kDisplaySerialNumber)), CGDisplaySerialNumber(display_id), 8)) {
        return -1;
      }

      return score > 0 ? score : -1;
    }

    /**
     * @brief Copy the best matching IOKit dictionary for a display.
     * @param display_id Display to query.
     * @return IOKit display dictionary, or null if unavailable.
     */
    [[nodiscard]] CfPtr<CFDictionaryRef> copyDisplayInfo(const MacDisplayId display_id) {
      auto matching {IOServiceMatching("IODisplayConnect")};
      if (!matching) {
        return {};
      }

      io_iterator_t iterator {IO_OBJECT_NULL};
      if (IOServiceGetMatchingServices(kIOMainPortDefault, matching, &iterator) != KERN_SUCCESS || iterator == IO_OBJECT_NULL) {
        return {};
      }

      IoObject iterator_guard {iterator};
      CfPtr<CFDictionaryRef> best_dictionary;
      int best_score {-1};

      while (const auto service = IOIteratorNext(iterator)) {
        IoObject service_guard {service};
        CfPtr<CFDictionaryRef> dictionary {IODisplayCreateInfoDictionary(service, kIODisplayOnlyPreferredName)};
        if (!dictionary) {
          continue;
        }

        const auto score {getMatchScore(dictionary.get(), display_id)};
        if (score > best_score) {
          best_score = score;
          best_dictionary.reset(static_cast<CFDictionaryRef>(CFRetain(dictionary.get())));
        }
      }

      return best_dictionary;
    }

    /**
     * @brief Build metadata text from an IOKit dictionary.
     * @param dictionary Dictionary to query.
     * @return Metadata string.
     */
    [[nodiscard]] std::string makeIokitMetadata(CFDictionaryRef dictionary) {
      if (!dictionary) {
        return {};
      }

      std::ostringstream metadata;
      metadata << "vendor=" << getUInt32(dictionary, CFSTR(kDisplayVendorID)).value_or(0) << ';'
               << "product=" << getUInt32(dictionary, CFSTR(kDisplayProductID)).value_or(0) << ';'
               << "serial=" << getUInt32(dictionary, CFSTR(kDisplaySerialNumber)).value_or(0) << ';'
               << "serial_string=" << getString(dictionary, CFSTR(kDisplaySerialString)) << ';'
               << "location=" << getString(dictionary, CFSTR(kIODisplayLocationKey)) << ';'
               << "name=" << getProductName(dictionary);
      return metadata.str();
    }

    /**
     * @brief Calculate FNV-1a hash for a byte range.
     * @param bytes Bytes to hash.
     * @return Hash value.
     */
    [[nodiscard]] std::uint64_t hashBytes(const std::vector<std::byte> &bytes) {
      std::uint64_t hash {14695981039346656037ULL};
      for (const auto byte : bytes) {
        hash ^= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(byte));
        hash *= 1099511628211ULL;
      }

      return hash;
    }

    /**
     * @brief Calculate FNV-1a hash for text.
     * @param text Text to hash.
     * @return Hash value.
     */
    [[nodiscard]] std::uint64_t hashText(const std::string &text) {
      std::vector<std::byte> bytes;
      bytes.reserve(text.size());
      for (const auto character : text) {
        bytes.push_back(static_cast<std::byte>(static_cast<unsigned char>(character)));
      }

      return hashBytes(bytes);
    }

    /**
     * @brief Format a device id.
     * @param prefix Prefix describing the id source.
     * @param hash Hash value.
     * @return Formatted device id.
     */
    [[nodiscard]] std::string makeDeviceId(const std::string_view prefix, const std::uint64_t hash) {
      std::ostringstream output;
      output << "macos-" << prefix << '-' << std::hex << std::setw(16) << std::setfill('0') << hash;
      return output.str();
    }
  }  // namespace

  bool MacApiLayer::isApiAccessAvailable() const {
    std::uint32_t display_count {0};
    return CGGetActiveDisplayList(0, nullptr, &display_count) == kCGErrorSuccess;
  }

  std::string MacApiLayer::getErrorString(const MacApiError error_code) const {
    std::ostringstream error;
    error << "[code: " << error_code << "] ";

    switch (error_code) {
      case kCGErrorSuccess:
        error << "Success";
        break;
      case kCGErrorFailure:
        error << "Failure";
        break;
      case kCGErrorIllegalArgument:
        error << "Illegal argument";
        break;
      case kCGErrorInvalidConnection:
        error << "Invalid connection";
        break;
      case kCGErrorInvalidContext:
        error << "Invalid context";
        break;
      case kCGErrorCannotComplete:
        error << "Cannot complete";
        break;
      case kCGErrorNotImplemented:
        error << "Not implemented";
        break;
      case kCGErrorRangeCheck:
        error << "Range check failed";
        break;
      case kCGErrorTypeCheck:
        error << "Type check failed";
        break;
      case kCGErrorInvalidOperation:
        error << "Invalid operation";
        break;
      case kCGErrorNoneAvailable:
        error << "None available";
        break;
      default:
        error << "Unknown CoreGraphics error";
        break;
    }

    return error.str();
  }

  MacDisplayIdList MacApiLayer::getDisplayIds(const MacQueryType type) const {
    using GetDisplayListFn = CGError (*)(std::uint32_t, CGDirectDisplayID *, std::uint32_t *);

    const GetDisplayListFn get_display_list {type == MacQueryType::Active ? CGGetActiveDisplayList : CGGetOnlineDisplayList};

    std::uint32_t display_count {0};
    if (get_display_list(0, nullptr, &display_count) != kCGErrorSuccess || display_count == 0) {
      return {};
    }

    MacDisplayIdList displays(display_count);
    if (get_display_list(display_count, displays.data(), &display_count) != kCGErrorSuccess) {
      return {};
    }

    displays.resize(display_count);
    return displays;
  }

  std::string MacApiLayer::getDeviceId(const MacDisplayId display_id) const {
    const auto edid {getEdid(display_id)};
    if (!edid.empty()) {
      return makeDeviceId("edid", hashBytes(edid));
    }

    if (const auto dictionary {copyDisplayInfo(display_id)}) {
      if (const auto metadata {makeIokitMetadata(dictionary.get())}; !metadata.empty()) {
        return makeDeviceId("iokit", hashText(metadata));
      }
    }

    std::ostringstream fallback;
    fallback << "vendor=" << CGDisplayVendorNumber(display_id) << ';'
             << "model=" << CGDisplayModelNumber(display_id) << ';'
             << "serial=" << CGDisplaySerialNumber(display_id) << ';'
             << "unit=" << CGDisplayUnitNumber(display_id) << ';'
             << "display=" << display_id;
    return makeDeviceId("cg", hashText(fallback.str()));
  }

  std::optional<MacDisplayMode> MacApiLayer::getCurrentDisplayMode(const MacDisplayId display_id) const {
    const auto mode {CGDisplayCopyDisplayMode(display_id)};
    if (!mode) {
      return std::nullopt;
    }

    const auto result {toDisplayMode(mode)};
    CGDisplayModeRelease(mode);
    return result;
  }

  MacDisplayModeList MacApiLayer::getDisplayModes(const MacDisplayId display_id) const {
    CfPtr<CFArrayRef> modes_ref {CGDisplayCopyAllDisplayModes(display_id, nullptr)};
    if (!modes_ref) {
      return {};
    }

    MacDisplayModeList modes;
    const auto mode_count {CFArrayGetCount(modes_ref.get())};
    for (CFIndex index = 0; index < mode_count; ++index) {
      const auto mode {static_cast<CGDisplayModeRef>(const_cast<void *>(CFArrayGetValueAtIndex(modes_ref.get(), index)))};
      if (!mode || !CGDisplayModeIsUsableForDesktopGUI(mode)) {
        continue;
      }

      if (const auto converted_mode {toDisplayMode(mode)}; converted_mode && std::ranges::find(modes, *converted_mode) == std::end(modes)) {
        modes.push_back(*converted_mode);
      }
    }

    return modes;
  }

  std::string MacApiLayer::getDisplayName(const MacDisplayId display_id) const {
    std::ostringstream name;
    name << "macos-display-" << display_id;
    return name.str();
  }

  std::string MacApiLayer::getFriendlyName(const MacDisplayId display_id) const {
    if (const auto dictionary {copyDisplayInfo(display_id)}) {
      return getProductName(dictionary.get());
    }

    return {};
  }

  std::vector<std::byte> MacApiLayer::getEdid(const MacDisplayId display_id) const {
    const auto dictionary {copyDisplayInfo(display_id)};
    if (!dictionary) {
      return {};
    }

    const auto edid {static_cast<CFDataRef>(getTypedValue(dictionary.get(), CFSTR(kIODisplayEDIDKey), CFDataGetTypeID()))};
    if (!edid) {
      return {};
    }

    const auto length {CFDataGetLength(edid)};
    const auto *bytes {CFDataGetBytePtr(edid)};
    if (!bytes || length <= 0) {
      return {};
    }

    std::vector<std::byte> result(static_cast<std::size_t>(length));
    std::ranges::transform(bytes, bytes + length, std::begin(result), [](const auto byte) {
      return static_cast<std::byte>(byte);
    });
    return result;
  }

  std::optional<Rational> MacApiLayer::getDisplayScale(const MacDisplayId display_id) const {
    const auto bounds {CGDisplayBounds(display_id)};
    if (bounds.size.width <= 0.) {
      return std::nullopt;
    }

    const auto pixel_width {CGDisplayPixelsWide(display_id)};
    if (pixel_width == 0) {
      return std::nullopt;
    }

    const auto scale {static_cast<double>(pixel_width) / bounds.size.width};
    return Rational {static_cast<unsigned int>(std::llround(scale * 100.)), 100};
  }

  std::optional<Point> MacApiLayer::getOriginPoint(const MacDisplayId display_id) const {
    const auto bounds {CGDisplayBounds(display_id)};
    return Point {
      static_cast<int>(std::llround(bounds.origin.x)),
      static_cast<int>(std::llround(bounds.origin.y))
    };
  }

  bool MacApiLayer::isMainDisplay(const MacDisplayId display_id) const {
    return CGDisplayIsMain(display_id) != 0;
  }

  bool MacApiLayer::isActive(const MacDisplayId display_id) const {
    return CGDisplayIsActive(display_id) != 0;
  }

  bool MacApiLayer::isOnline(const MacDisplayId display_id) const {
    return CGDisplayIsOnline(display_id) != 0;
  }

  MacDisplayId MacApiLayer::getMirrorMaster(const MacDisplayId display_id) const {
    return CGDisplayMirrorsDisplay(display_id);
  }

  bool MacApiLayer::setDisplayMode(const MacDisplayId display_id, const MacDisplayMode &mode) {
    static_cast<void>(display_id);
    static_cast<void>(mode);
    return false;
  }

  bool MacApiLayer::setOriginPoint(const MacDisplayId display_id, const Point &origin) {
    static_cast<void>(display_id);
    static_cast<void>(origin);
    return false;
  }

  bool MacApiLayer::setMirror(const MacDisplayId display_id, const MacDisplayId master_display_id) {
    static_cast<void>(display_id);
    static_cast<void>(master_display_id);
    return false;
  }
}  // namespace display_device
