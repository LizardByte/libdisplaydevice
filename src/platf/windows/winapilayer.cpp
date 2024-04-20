// class header include
#include "winapilayer.h"

// system includes
#include <iomanip>

// local includes
#include "src/logging.h"

namespace display_device {
  namespace {
    /** @brief Dumps the result of @see query_display_config into a string */
    std::string
    dump_path(const DISPLAYCONFIG_PATH_INFO &info) {
      std::ostringstream output;
      std::ios state(nullptr);
      state.copyfmt(output);

      // clang-format off
      output << "sourceInfo:" << std::endl;
      output << "    adapterId: [" << info.sourceInfo.adapterId.HighPart << ", " << info.sourceInfo.adapterId.LowPart << "]" << std::endl;
      output << "    id: " << info.sourceInfo.id << std::endl;
      output << "        cloneGroupId: " << info.sourceInfo.cloneGroupId << std::endl;
      output << "        sourceModeInfoIdx: " << info.sourceInfo.sourceModeInfoIdx << std::endl;
      output << "        modeInfoIdx: " << info.sourceInfo.modeInfoIdx << std::endl;
      output << "    statusFlags: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.sourceInfo.statusFlags << std::endl;
      output.copyfmt(state);
      output << "targetInfo:" << std::endl;
      output << "    adapterId: [" << info.targetInfo.adapterId.HighPart << ", " << info.targetInfo.adapterId.LowPart << "]" << std::endl;
      output << "    id: " << info.targetInfo.id << std::endl;
      output << "        desktopModeInfoIdx: " << info.targetInfo.desktopModeInfoIdx << std::endl;
      output << "        targetModeInfoIdx: " << info.targetInfo.targetModeInfoIdx << std::endl;
      output << "        modeInfoIdx: " << info.targetInfo.modeInfoIdx << std::endl;
      output << "    outputTechnology:  0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.outputTechnology << std::endl;
      output << "    rotation: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.rotation << std::endl;
      output << "    scaling: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.scaling << std::endl;
      output.copyfmt(state);
      output << "    refreshRate: " << info.targetInfo.refreshRate.Numerator << "/" << info.targetInfo.refreshRate.Denominator << std::endl;
      output << "    scanLineOrdering: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.scanLineOrdering << std::endl;
      output << "    targetAvailable: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.targetAvailable << std::endl;
      output << "    statusFlags: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.targetInfo.statusFlags << std::endl;
      output << "flags: 0x" << std::uppercase << std::setfill('0') << std::setw(8) << std::hex << info.flags;
      // clang-format on

      return output.str();
    }

    /** @brief Dumps the result of @see query_display_config into a string */
    std::string
    dump_mode(const DISPLAYCONFIG_MODE_INFO &info) {
      std::stringstream output;
      std::ios state(nullptr);
      state.copyfmt(output);

      if (info.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {
        // clang-format off
        output << "width: " << info.sourceMode.width << std::endl;
        output << "height: " << info.sourceMode.height << std::endl;
        output << "pixelFormat: " << info.sourceMode.pixelFormat << std::endl;
        output << "position: [" << info.sourceMode.position.x << ", " << info.sourceMode.position.y << "]";
        // clang-format on
      }
      else if (info.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_TARGET) {
        // clang-format off
        output << "pixelRate: " << info.targetMode.targetVideoSignalInfo.pixelRate << std::endl;
        output << "hSyncFreq: " << info.targetMode.targetVideoSignalInfo.hSyncFreq.Numerator << "/" << info.targetMode.targetVideoSignalInfo.hSyncFreq.Denominator << std::endl;
        output << "vSyncFreq: " << info.targetMode.targetVideoSignalInfo.vSyncFreq.Numerator << "/" << info.targetMode.targetVideoSignalInfo.vSyncFreq.Denominator << std::endl;
        output << "activeSize: [" << info.targetMode.targetVideoSignalInfo.activeSize.cx << ", " << info.targetMode.targetVideoSignalInfo.activeSize.cy << "]" << std::endl;
        output << "totalSize: [" << info.targetMode.targetVideoSignalInfo.totalSize.cx << ", " << info.targetMode.targetVideoSignalInfo.totalSize.cy << "]" << std::endl;
        output << "videoStandard: " << info.targetMode.targetVideoSignalInfo.videoStandard << std::endl;
        output << "scanLineOrdering: " << info.targetMode.targetVideoSignalInfo.scanLineOrdering;
        // clang-format on
      }
      else if (info.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_DESKTOP_IMAGE) {
        // TODO: One day MinGW will add updated struct definition and the following code can be enabled
        // clang-format off
        // output << "PathSourceSize: [" << info.desktopImageInfo.PathSourceSize.x << ", " << info.desktopImageInfo.PathSourceSize.y << "]" << std::endl;
        // output << "DesktopImageRegion: [" << info.desktopImageInfo.DesktopImageRegion.bottom << ", " << info.desktopImageInfo.DesktopImageRegion.left << ", " << info.desktopImageInfo.DesktopImageRegion.right << ", " << info.desktopImageInfo.DesktopImageRegion.top << "]" << std::endl;
        // output << "DesktopImageClip: [" << info.desktopImageInfo.DesktopImageClip.bottom << ", " << info.desktopImageInfo.DesktopImageClip.left << ", " << info.desktopImageInfo.DesktopImageClip.right << ", " << info.desktopImageInfo.DesktopImageClip.top << "]";
        // clang-format on
        output << "NOT SUPPORTED BY COMPILER YET...";
      }
      else {
        output << "NOT IMPLEMENTED YET...";
      }

      return output.str();
    }

    /** @brief Dumps the result of @see query_display_config into a string */
    std::string
    dump_paths_and_modes(const std::vector<DISPLAYCONFIG_PATH_INFO> &paths,
      const std::vector<DISPLAYCONFIG_MODE_INFO> &modes) {
      std::ostringstream output;

      output << std::endl
             << "Got " << paths.size() << " path(s):";
      bool path_dumped { false };
      for (auto i { 0u }; i < paths.size(); ++i) {
        output << std::endl
               << "----------------------------------------[index: " << i << "]" << std::endl;

        output << dump_path(paths[i]);
        path_dumped = true;
      }

      if (path_dumped) {
        output << std::endl
               << std::endl;
      }

      output << "Got " << modes.size() << " mode(s):";
      for (auto i { 0u }; i < modes.size(); ++i) {
        output << std::endl
               << "----------------------------------------[index: " << i << "]" << std::endl;

        output << dump_mode(modes[i]);
      }

      return output.str();
    }
  }  // namespace

  std::string
  WinApiLayer::get_error_string(LONG error_code) const {
    std::ostringstream error;
    error << "[code: ";
    switch (error_code) {
      case ERROR_INVALID_PARAMETER:
        error << "ERROR_INVALID_PARAMETER";
        break;
      case ERROR_NOT_SUPPORTED:
        error << "ERROR_NOT_SUPPORTED";
        break;
      case ERROR_ACCESS_DENIED:
        error << "ERROR_ACCESS_DENIED";
        break;
      case ERROR_INSUFFICIENT_BUFFER:
        error << "ERROR_INSUFFICIENT_BUFFER";
        break;
      case ERROR_GEN_FAILURE:
        error << "ERROR_GEN_FAILURE";
        break;
      case ERROR_SUCCESS:
        error << "ERROR_SUCCESS";
        break;
      default:
        error << error_code;
        break;
    }
    error << ", message: " << std::system_category().message(static_cast<int>(error_code)) << "]";
    return error.str();
  }

  std::optional<WinApiLayerInterface::path_and_mode_data_t>
  WinApiLayer::query_display_config(query_type_e type) const {
    std::vector<DISPLAYCONFIG_PATH_INFO> paths;
    std::vector<DISPLAYCONFIG_MODE_INFO> modes;
    LONG result = ERROR_SUCCESS;

    // When we want to enable/disable displays, we need to get all paths as they will not be active.
    // This will require some additional filtering of duplicate and otherwise useless paths.
    UINT32 flags = type == query_type_e::Active ? QDC_ONLY_ACTIVE_PATHS : QDC_ALL_PATHS;
    flags |= QDC_VIRTUAL_MODE_AWARE;  // supported from W10 onwards

    do {
      UINT32 path_count { 0 };
      UINT32 mode_count { 0 };

      result = GetDisplayConfigBufferSizes(flags, &path_count, &mode_count);
      if (result != ERROR_SUCCESS) {
        DD_LOG(error) << get_error_string(result) << " failed to get display paths and modes!";
        return std::nullopt;
      }

      paths.resize(path_count);
      modes.resize(mode_count);
      result = QueryDisplayConfig(flags, &path_count, paths.data(), &mode_count, modes.data(), nullptr);

      // The function may have returned fewer paths/modes than estimated
      paths.resize(path_count);
      modes.resize(mode_count);

      // It's possible that between the call to GetDisplayConfigBufferSizes and QueryDisplayConfig
      // that the display state changed, so loop on the case of ERROR_INSUFFICIENT_BUFFER.
    } while (result == ERROR_INSUFFICIENT_BUFFER);

    if (result != ERROR_SUCCESS) {
      DD_LOG(error) << get_error_string(result) << " failed to query display paths and modes!";
      return std::nullopt;
    }

    DD_LOG(verbose) << "Result of " << (type == query_type_e::Active ? "ACTIVE" : "ALL") << " display config query:\n"
                    << dump_paths_and_modes(paths, modes) << "\n";
    return path_and_mode_data_t { paths, modes };
  }

}  // namespace display_device