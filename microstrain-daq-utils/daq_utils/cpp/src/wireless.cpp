#include <iostream>
#include <ostream>
#include <string>

#include <opendaq/opendaq.h>

#include <daq_utils/wireless.h>

namespace daq_utils::wireless {

void ListNodes(daq::DevicePtr baseStation)
{
    ListNodes(baseStation, std::cout);
}

void ListNodes(daq::DevicePtr baseStation, std::ostream& out)
{
    daq::ListPtr<daq::IChannel> allChannels = baseStation.getChannels();

    std::vector<daq::ChannelPtr> nodes;
    for (daq::ChannelPtr channel : allChannels)
    {
        std::string localId = channel.getLocalId();
        if (localId.rfind("node_", 0) == 0)
        {
            nodes.push_back(channel);
        }
    }

    const std::string col0Header = "#";
    const std::string col1Header = "Model (Node ID)";
    size_t col0 = col0Header.size();
    size_t col1 = col1Header.size();

    for (size_t i = 0; i < nodes.size(); ++i)
    {
        col0 = std::max(col0, std::to_string(i).size());
        col1 = std::max(col1, std::string(nodes[i].getName()).size());
    }

    auto pad = [](const std::string& s, size_t w) { return s + std::string(w - s.size(), ' '); };

    out << "\n";
    out << pad(col0Header, col0) << " | " << pad(col1Header, col1) << "\n";
    out << std::string(col0, '-') << "-+-" << std::string(col1, '-') << "\n";
    for (size_t i = 0; i < nodes.size(); ++i)
    {
        out << pad(std::to_string(i), col0) << " | " << std::string(nodes[i].getName()) << "\n";
    }
    out << "\n";
}

} // namespace daq_utils::wireless
