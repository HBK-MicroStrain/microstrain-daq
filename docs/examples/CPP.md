## Wireless C++ Examples

This document provides examples of common operations with wireless devices using the C++ API.

### Pinging Nodes

The ping command is used to check if there is proper communication between the base station and the node.

```cpp
daq::BaseObjectPtr result = daq_utils::Call(node, "Control.Ping");

if (result.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
{

    // Get some details from the response
    std::cout << "Successfully pinged Node" << node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.NodeAddress") << "\n";
    std::cout << "Base Station RSSI: " << result.asPtr<daq::IPropertyObject>().getPropertyValue("BaseRssi") << "\n";
    std::cout << "Node RSSI: " << result.asPtr<daq::IPropertyObject>().getPropertyValue("NodeRssi") << "\n";

    // We can talk to the Node, so let's get some more info
    std::cout << "\nNode Information: " << "\n";
    std::cout << "--------------------" << "\n";
    std::cout << "Model Number :" << node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.ModelName") << "\n";
    std::cout << "Serial: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.Serial") << "\n";
    std::cout << "Firmware: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.FirmwareVersion") << "\n";
}
else
{
    std::cout << "Failed to ping Node" << node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.NodeAddress") << "\n";
}
```
> **Note: To communicate with a Wireless Node, all the following must be true:**
>
>    - The Node is powered on, and within range of the Base Station
>    - The Node is on the same frequency as the Base Station
>    - The Node is in Idle Mode (not sampling, and not sleeping)
>    - The Node is on the same communication protocol as the Base Station (LXRS vs LXRS+)

### Setting to Idle

The **Set to Idle** command is used to put a Node that is sampling, or sleeping, back into the Idle Mode so that it may be communicated with.

```cpp
// The argument is the timeout in milliseconds
daq::BaseObjectPtr result = daq_utils::Call(node, "Control.SetToIdle", 8000);

std::cout << "Setting Node to Idle" << "\n";

std::string complete = result.asPtr<daq::IPropertyObject>().getPropertyValue("Complete");
std::string status = result.asPtr<daq::IPropertyObject>().getPropertyValue("Status");
std::string success = result.asPtr<daq::IPropertyObject>().getPropertyValue("Success");
std::cout << "Status: [Complete=" << complete << ", Status=" << status << ", Success=" << success << "]" << "\n";
```

### Getting Current Configuration Settings

Configuration indicates how nodes are set up for data acquisition. It includes settings such as sampling mode/rate, offsets, hardware gain, etc.

To read current configuration settings on the node:

```cpp
std::cout << "Current Configuration Settings" << "\n";

// Read some of the current configuration settings on the node
std::cout << "# of Triggers: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Setup.DataManagement.NumDatalogSessions") << "\n";
std::cout << "User Inactivity Timeout: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Setup.Configure.Power.InactivityTimeout") << " seconds" << "\n";
std::cout << "Total active channels: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Control.Sample.ActiveChannels") << "\n";
std::cout << "# of sweeps: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Control.Sample.NumSweeps") << "\n";
```

If a configuration function requires a channel mask parameter, this indicates that the option may affect one or more channels on the Node. You can either:

- Provide the channel mask when asking for the configuration (if known beforehand)
- Programmatically determine the mask for each setting

#### Programmatically Determining The Mask For Each Setting

```cpp
daq_utils::DaqTypeFactory daqTypes(instance);

daq::BaseObjectPtr chGroups = daq_utils::Call(node, "Capabilities.ChannelGroups");
daq::ListPtr<daq::IBaseObject> groupsList = chGroups.asPtr<daq::IPropertyObject>().getPropertyValue("Result");
daq::EnumerationPtr linearEqSetting = daqTypes.MakeEnum("ChannelGroupSetting", "chSetting_linearEquation");

for (size_t i = 0; i < groupsList.getCount(); ++i)
{
    daq::StructPtr group = groupsList[i];
    daq::ListPtr<daq::IBaseObject> settingsList = group.get("Settings");

    for (size_t j = 0; j < settingsList.getCount(); ++j)
    {
        // If the group contains the linear equation setting
        if (settingsList[j] == linearEqSetting)
        {
            // We can now pass the channel mask for this group to the function.
            // Note: once this channel mask is known for a specific node (+ fw version), it should never change
            daq::BaseObjectPtr eq = daq_utils::Call(node,"Setup.Configure.Calibration.GetLinearEquation",{group.get("Mask")});

            std::cout << "Linear Equation for: " << group.get("Name") << "\n";
            std::cout << "Slope: " << eq.asPtr<daq::IPropertyObject>().getPropertyValue("Slope") << "\n";
            std::cout << "Offset: " << eq.asPtr<daq::IPropertyObject>().getPropertyValue("Offset") << "\n";
        }
    }
}
```

### Setting Current Configuration Settings

To set current configuration settings for a node:

```cpp
daq_utils::DaqTypeFactory daqTypes(instance);

std::cout << "\nChanging configuration settings..." << "\n";

// Set the configuration options that we want to change
node.asPtr<daq::IPropertyObject>().setPropertyValue("Setup.Configure.Power.DefaultMode", daqTypes.MakeEnum("DefaultMode", "defaultMode_idle"));
node.asPtr<daq::IPropertyObject>().setPropertyValue("Setup.Configure.Power.InactivityTimeout", 7200);
node.asPtr<daq::IPropertyObject>().setPropertyValue("Control.Sample.SamplingMode", daqTypes.MakeEnum("SamplingMode", "samplingMode_sync"));
node.asPtr<daq::IPropertyObject>().setPropertyValue("Control.Sample.SampleRate", daqTypes.MakeEnum("WirelessSampleRate", "sampleRate_256Hz"));
node.asPtr<daq::IPropertyObject>().setPropertyValue("Control.Sample.UnlimitedDuration", true);

// Attempt to verify the configuration with the Node we want to apply it to
//  Note: This step is not required before applying; however, the apply will throw an
//        Error_InvalidNodeConfig exception if the config fails to verify.
daq::BaseObjectPtr verification = daq_utils::Call(node, "Setup.Configure.Verify");

if (!verification.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
{
    std::cout << "\nFailed to verify the configuration. The following issues were found:" << "\n";
    // Print out all the issues that were found
    std::cout << verification.asPtr<daq::IPropertyObject>().getPropertyValue("Issues") << "\n";
    std::cout << "Configuration will not be applied." << "\n";
}
else
{
    // Apply the configuration to the Node
    // Note: This writes multiple options to the Node. If an Error_NodeCommunication
    //       exception is thrown, it is possible that some options were successfully
    //       applied, while others failed. It is recommended to keep calling
    //       Apply until no exception is thrown.
    daq::BaseObjectPtr application = daq_utils::Call(node, "Setup.Configure.Apply");
    if (!application.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
    {
        // Print out all the issues that were found
        std::cout << application.asPtr<daq::IPropertyObject>().getPropertyValue("Issues") << "\n";
        std::cout << "Application failed." << "\n";
    }
}

std::cout << "Done." << "\n";
```

### Starting Sync Sampling

Synchronized Sampling is a sampling mode that automatically coordinates all incoming node data to a particular gateway. It is designed to ensure data arrival and sequence.

This code snippet provides the function to start sync sampling:

```cpp
// Select nodes
for (const daq::ChannelPtr& node : base_station.getChannels())
{
    std::cout << "Adding node" << node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.NodeAddress") << "\n";

    // Enable the node for sampling
    node.asPtr<daq::IPropertyObject>().setPropertyValue("Control.Sample.Enabled", true);

    // Add the node to the network
    daq::BaseObjectPtr add_result = daq_utils::Call(base_station, "Control.Sample.AddNode", {node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.NodeAddress")});
    if (!add_result.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
    {
        std::cout << "Adding node failed:" << add_result.asPtr<daq::IPropertyObject>().getPropertyValue("Error") << "\n";
    }
}

// Can get information about the network
std::cout << "Network info:" << "\n";
std::cout << "Network OK: " << base_station.asPtr<daq::IPropertyObject>().getPropertyValue("Control.Sample.IsValid") << "\n";
std::cout << "Percent of Bandwidth: " << base_station.asPtr<daq::IPropertyObject>().getPropertyValue("Control.Sample.NetworkBandwidth") << "\n";
std::cout << "Lossless Enabled: " << base_station.asPtr<daq::IPropertyObject>().getPropertyValue("Control.Sample.Lossless") << "\n";

// Start the network once all nodes have been added
daq::BaseObjectPtr start_result = daq_utils::Call(base_station, "Control.Sample.ApplyAndStartNetwork");
std::cout << "Start network succeeded: " << start_result.asPtr<daq::IPropertyObject>().getPropertyValue("Success") << "\n";
```

> Note: The Nodes must already be configured for Sync Sampling before adding to the network, or else Error_InvalidNodeConfig will be thrown.
>
> If you wish to provide your own start time (instead of using the system time), pass a Timestamp object as a second parameter to this function.
>
> If you don't want to enable a beacon at this time, use the `Control.Sample.ApplyAndArmNodes` function. The nodes will wait until they hear a beacon to start sampling.

### Enabling Beacons

The beacon is used to synchronize and start a group of nodes when performing Synchronized Sampling.

To enable a beacon:

```cpp
// Make sure we can ping the base station
daq::PropertyObjectPtr enableBeaconResult = daq_utils::Call(base_station, "Control.EnableBeacon").asPtr<daq::IPropertyObject>();
daq::BaseObjectPtr beaconTime = enableBeaconResult.getPropertyValue("Timestamp");

if (!enableBeaconResult.getPropertyValue("Success"))
{
    std::cout << "Failed to ping the Base Station" << "\n";
}

if (base_station.asPtr<daq::IPropertyObject>().getPropertyValue("Capabilities.SupportsBeaconStatus"))
{
    daq::BaseObjectPtr status = daq_utils::Call(base_station, "Control.GetBeaconStatus");
    std::cout << "Beacon current status: Enabled?: " << (status.asPtr<daq::IPropertyObject>().getPropertyValue("Enabled") ? "TRUE" : "FALSE") << "\n";
    std::cout << "Time: " << status.asPtr<daq::IPropertyObject>().getPropertyValue("Timestamp") << "\n";
}

std::cout << "Attempting to enable the beacon..." << "\n";

// Enable the beacon on the Base Station using the PC time
if (!enableBeaconResult.getPropertyValue("Success"))
{
    std::cout << "Failed to enable the beacon" << "\n";
}
else
{
    // If we got here, no exception was thrown, so enableBeacon was successful
    std::cout << "Successfully enabled the beacon on the Base Station" << "\n";
    std::cout << "Beacon's initial Timestamp: " << beaconTime << "\n";
    std::cout << "Beacon is active" << "\n";
}
```

### Disabling Beacons

To disable a beacon:

```cpp
// Disable the beacon on the Base Station
daq::BaseObjectPtr disableBeaconResult = daq_utils::Call(base_station, "Control.DisableBeacon").asPtr<daq::IPropertyObject>();

if (!disableBeaconResult.getPropertyValue("Success"))
{
    std::cout << "Failed to disable the beacon" << "\n";
}
else
{
    // If we got here, disableBeacon was successful
    std::cout << "Successfully disabled the beacon on the Base Station" << "\n";
}
```

### Streaming Data

The openDAQ module exposes each physical measurement channel as a separate signal. Data is read using a stream reader, which buffers incoming samples and lets you consume them at your own pace.

```cpp
#include <vector>
#include <thread>
#include <chrono>

// openDAQ delivers each measurement channel through its own signal, so a
// separate StreamReader is needed for each one. Readers must be created once
// before the read loop. Creating them inside the loop would discard any samples
// buffered since the previous iteration.
//
// Signals only exist after the node has sent its first packet of data, so this
// list will be empty if sampling has not started yet. Make sure the network is
// running before building it.
struct NodeSignalReader
{
    std::string node_address;
    std::string channel_name;
    daq::StreamReaderPtr reader;
};

std::vector<NodeSignalReader> node_signal_readers;

void BuildLiveReaders(const daq::ChannelPtr& base_station, std::vector<NodeSignalReader>& node_signal_readers)
{
    node_signal_readers.clear();

    for (const daq::ChannelPtr& node : base_station.getChannels())
    {
        std::string node_address = node.asPtr<daq::IPropertyObject>().getPropertyValue("Advanced.NodeAddress");
        daq::ListPtr<daq::ISignal> signals = node.getSignalsRecursive();

        for (const daq::SignalPtr& signal : signals)
        {
            std::string signalName = signal.getName();

            if (signalName.rfind("DataLog_ch", 0) == 0)
                continue;

            if (!signal.getDomainSignal().assigned())
                continue;

            daq::StreamReaderPtr reader = daq::StreamReader<double, uint64_t>(signal);
            node_signal_readers.push_back({node_address, signalName, reader});
        }
    }
}

BuildLiveReaders(base_station, node_signal_readers);

for (int retry = 0; retry < 20 && node_signal_readers.empty(); ++retry)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    BuildLiveReaders(base_station, node_signal_readers);
}

    std::cout << "Readers created: " << node_signal_readers.size() << "\n";
    if (node_signal_readers.empty())
    {
        std::cout << "No live measurement signals found. Ensure sampling network is started and node channels are enabled.\n";
        return 1;
    }

// Poll for new samples
while (true)
{
    for (NodeSignalReader& item : node_signal_readers)
    {
        daq::SizeT count = 256;
        std::vector<double> values(count);
        std::vector<uint64_t> timestamps(count);
        item.reader.readWithDomain(values.data(), timestamps.data(), &count);

        if (count > 0)
        {
            for (daq::SizeT i = 0; i < count; ++i)
            {
                std::cout << "Node " << item.node_address << " | " << item.channel_name
                          << ": " << values[i] << " @ " << timestamps[i] << "\n";
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
}
```

> **Note:** Each measurement channel is delivered independently through its own signal. Samples taken at the same time across multiple channels on a node will share the same timestamp value.

### Downloading Logged Data

A Node can keep recording to its own internal memory even if it loses connection to the Base Station. That data can be downloaded into openDAQ signals just like live data.

To view the current datalogging status for a node:

```cpp
std::cout << "Datalog sessions stored on node: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Setup.DataManagement.NumDatalogSessions") << "\n";
std::cout << "Node storage full: " << node.asPtr<daq::IPropertyObject>().getPropertyValue("Setup.DataManagement.PercentFull") << "\n";
```

Before starting a download, call `PrepareDownload` to pre-create the datalog signals so readers can be attached to them.

> Note: If no reader is attached before `DownloadData` is called, the download will fail with an error.

```cpp
daq::BaseObjectPtr prepare_result = daq_utils::Call(node, "Setup.DataManagement.PrepareDownload");

if (!prepare_result.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
{
    std::cout << "Failed to prepare for download" << "\n";
}
```

Each logged channel is exposed as its own signal, named with a "DataLog_ch" prefix. Once the download is prepared, attach readers to each:

```cpp
struct DatalogReader
{
    std::string channel_name;
    daq::StreamReaderPtr reader;
};

std::vector<DatalogReader> datalog_readers;

for (const daq::SignalPtr& signal : node.getSignals(daq::RecursiveSearchFilter(daq::AnySearchFilter())))
{
    std::string name = signal.getName();
    if (name.rfind("DataLog_ch", 0) == 0)
    {
        daq::StreamReaderPtr reader = daq::StreamReader<double, uint64_t>(signal);
        datalog_readers.push_back({name, reader});
    }
}
```

With readers in place, start the download:

```cpp
// The argument is the number of retries allowed per sweep before the download is aborted
daq::BaseObjectPtr download_result = daq_utils::Call(node, "Setup.DataManagement.DownloadData", {3});

if (!download_result.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
{
    std::cout << "Failed to start download" << "\n";
}
else
{
    std::cout << "Download started" << "\n";
}
```

The download runs asynchronously, so `DownloadData` returns immediately and progress must be polled separately:

```cpp
// Poll for progress until the download reaches a terminal state
while (true)
{
    daq::BaseObjectPtr progress = daq_utils::Call(node, "Setup.DataManagement.DownloadProgress");
    std::string state = progress.asPtr<daq::IPropertyObject>().getPropertyValue("State");
    std::cout << "State: " << state << ", Progress: "
              << progress.asPtr<daq::IPropertyObject>().getPropertyValue("Progress") << "%" << "\n";

    if (state == "Complete" || state == "Failed" || state == "Canceled")
        break;

    // Read out any samples that have arrived so far
    for (DatalogReader& [channel_name, reader] : datalog_readers)
    {
        daq::SizeT count = reader.getAvailableCount();

        if (count > 0)
        {
            std::vector<double> values(count);
            std::vector<uint64_t> timestamps(count);
            reader.readWithDomain(values.data(), timestamps.data(), &count);
            
            for (daq::SizeT i = 0; i < count; ++i)
            {
                std::cout << channel_name << ": " << values[i] << " @ " << timestamps[i] << "\n";
            }
        }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

To stop a download before it finishes, call `CancelDownload`:

```cpp
daq::BaseObjectPtr cancel_result = daq_utils::Call(node, "Setup.DataManagement.CancelDownload");

if (!cancel_result.asPtr<daq::IPropertyObject>().getPropertyValue("Success"))
{
    std::cout << "Failed to cancel download" << "\n";
}
```

> **Note:** A download can only be canceled while it is in progress. Calling `CancelDownload` when no download is running returns `Success` as False.