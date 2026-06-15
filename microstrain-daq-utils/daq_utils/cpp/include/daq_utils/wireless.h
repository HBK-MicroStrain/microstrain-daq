#pragma once

#include <iosfwd>

#include <opendaq/device_ptr.h>

namespace daq_utils::wireless {

/// @brief Prints a table of all wireless node channels discovered by a base station device.
/// @param baseStation The openDAQ device representing the wireless base station.
void ListNodes(daq::DevicePtr baseStation);

/// @brief Prints a table of all wireless node channels discovered by a base station device
/// to the given stream.
/// @param baseStation The openDAQ device representing the wireless base station.
/// @param out The output stream to write to.
void ListNodes(daq::DevicePtr baseStation, std::ostream& out);

} // namespace daq_utils::wireless
