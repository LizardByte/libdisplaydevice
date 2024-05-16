#pragma once

// system includes
#include <memory>

// local includes
#include "winapilayerinterface.h"
#include "windisplaydeviceinterface.h"

namespace display_device {
  /**
   * @brief Default implementation for the WinDisplayDeviceInterface.
   */
  class WinDisplayDevice: public WinDisplayDeviceInterface {
  public:
    /**
     * Default constructor for the class.
     * @param w_api A pointer to the Windows API layer. Will throw on nullptr!
     */
    explicit WinDisplayDevice(std::shared_ptr<WinApiLayerInterface> w_api);

    /** For details @see WinDisplayDevice::getCurrentTopology */
    [[nodiscard]] ActiveTopology
    getCurrentTopology() const override;

    /** For details @see WinDisplayDevice::isTopologyValid */
    [[nodiscard]] bool
    isTopologyValid(const ActiveTopology &topology) const override;

    /** For details @see WinDisplayDevice::getCurrentTopology */
    [[nodiscard]] bool
    isTopologyTheSame(const ActiveTopology &lhs, const ActiveTopology &rhs) const override;

    /** For details @see WinDisplayDevice::setTopology */
    [[nodiscard]] bool
    setTopology(const ActiveTopology &new_topology) override;

  private:
    std::shared_ptr<WinApiLayerInterface> m_w_api;
  };
}  // namespace display_device
