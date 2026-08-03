## Wireless C# Examples

This document provides examples of common operations with wireless devices using the C# API.

### Pinging Nodes

The ping command is used to check if there is proper communication between the base station and the node.

```c#
BaseObject? result = DaqUtils.Call(node, "Control.Ping");

if (result?.Cast<PropertyObject>().GetPropertyValue("Success"))
{

    // Get some details from the response
    Console.WriteLine($"Successfully pinged Node {node.Cast<PropertyObject>().GetPropertyValue("Advanced.NodeAddress")} \n");
    Console.WriteLine($"Base Station RSSI: {result.Cast<PropertyObject>().GetPropertyValue("BaseRssi")} \n");
    Console.WriteLine($"Node RSSI: {result.Cast<PropertyObject>().GetPropertyValue("NodeRssi")} \n");

    // We can talk to the Node, so let's get some more info
    Console.WriteLine("\nNode Information: \n");
    Console.WriteLine("--------------------\n");
    Console.WriteLine($"Model Number : {node.Cast<PropertyObject>().GetPropertyValue("Advanced.ModelName")} \n");
    Console.WriteLine($"Serial: {node.Cast<PropertyObject>().GetPropertyValue("Advanced.Serial")} \n");
    Console.WriteLine($"Firmware: {node.Cast<PropertyObject>().GetPropertyValue("Advanced.FirmwareVersion")} \n");
}
else
{
    Console.WriteLine($"Failed to ping Node {node.Cast<PropertyObject>().GetPropertyValue("Advanced.NodeAddress")} \n");
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

```c#
// The argument is the timeout in milliseconds
BaseObject? result = DaqUtils.Call(node, "Control.SetToIdle", 8000);

Console.WriteLine("Setting Node to Idle \n");

BaseObject? complete = result?.Cast<PropertyObject>().GetPropertyValue("Complete");
BaseObject? status = result?.Cast<PropertyObject>().GetPropertyValue("Status");
BaseObject? success = result?.Cast<PropertyObject>().GetPropertyValue("Success");
Console.WriteLine($"Status: [Complete= {complete}, Status= {status}, Success= {success}] \n");
```

### Getting Current Configuration Settings

Configuration indicates how nodes are set up for data acquisition. It includes settings such as sampling mode/rate, offsets, hardware gain, etc.

To read current configuration settings on the node:

```c#
Console.WriteLine("Current Configuration Settings \n");

// Read some of the current configuration settings on the node
Console.WriteLine($"# of Triggers: {node.Cast<PropertyObject>().GetPropertyValue("Setup.DataManagement.NumDatalogSessions")} \n");
Console.WriteLine($"User Inactivity Timeout: {node.Cast<PropertyObject>().GetPropertyValue("Setup.Configure.Power.InactivityTimeout")} seconds \n");
Console.WriteLine($"Total active channels: {node.Cast<PropertyObject>().GetPropertyValue("Control.Sample.ActiveChannels")} \n");
Console.WriteLine($"# of sweeps: {node.Cast<PropertyObject>().GetPropertyValue("Control.Sample.NumSweeps")} \n");
```

If a configuration function requires a channel mask parameter, this indicates that the option may affect one or more channels on the Node. You can either:

- Provide the channel mask when asking for the configuration (if known beforehand)
- Programmatically determine the mask for each setting

#### Programmatically Determining The Mask For Each Setting

```c#
DaqTypeFactory daqTypes = new DaqTypeFactory(instance);

BaseObject? chGroups = DaqUtils.Call(node, "Capabilities.ChannelGroups");
PropertyObject? chGroupsProps = chGroups?.Cast<PropertyObject>();
BaseObject? resultObj = chGroupsProps?.GetPropertyValue("Result");
ListObject<BaseObject>? groupsList = resultObj?.Cast<ListObject<BaseObject>>();

// chSetting_linearEquation is at index 3 in the enumeration
long linearEqValue = 3;

foreach (BaseObject groupItem in groupsList)
{

    Struct? group = groupItem.Cast<Struct>();
    BaseObject? settingsObj = group.Get("Settings");
    ListObject<BaseObject>? settingsList = settingsObj.Cast<ListObject<BaseObject>>();
    foreach (BaseObject setting in settingsList)
    {
        IntegerObject? intSetting = setting.Cast<IntegerObject>();
        long settingValue = intSetting != null ? (long)intSetting : -1;
        
        if (settingValue == linearEqValue)
        {
            IntegerObject? groupMask = group.Get("Mask")?.Cast<IntegerObject>();
            if (groupMask != null)
            {
                BaseObject? eq = DaqUtils.Call(node, "Setup.Configure.Calibration.GetLinearEquation", groupMask);
                Console.WriteLine($"Linear Equation for: {group.Get("Name")}");
                Console.WriteLine($"Slope: {eq?.Cast<PropertyObject>().GetPropertyValue("Slope")}");
                Console.WriteLine($"Offset: {eq?.Cast<PropertyObject>().GetPropertyValue("Offset")} \n");
            }
        }
    }
}
```

### Setting Current Configuration Settings

To set current configuration settings for a node:

```c#
DaqTypeFactory daqTypes = new DaqTypeFactory(instance);

Console.WriteLine("\nChanging configuration settings... \n");

// Set the configuration options that we want to change
node.Cast<PropertyObject>().SetPropertyValue("Setup.Configure.Power.DefaultMode", daqTypes.MakeEnum("mscl_WirelessTypes_DefaultMode", "defaultMode_idle"));
node.Cast<PropertyObject>().SetPropertyValue("Setup.Configure.Power.InactivityTimeout", 7200);
node.Cast<PropertyObject>().SetPropertyValue("Control.Sample.SamplingMode", daqTypes.MakeEnum("mscl_WirelessTypes_SamplingMode", "samplingMode_sync"));
node.Cast<PropertyObject>().SetPropertyValue("Control.Sample.SampleRate", daqTypes.MakeEnum("mscl_WirelessTypes_WirelessSampleRate", "sampleRate_256Hz"));
node.Cast<PropertyObject>().SetPropertyValue("Control.Sample.UnlimitedDuration", true);

// Attempt to verify the configuration with the Node we want to apply it to
//  Note: This step is not required before applying; however, the apply will throw an
//        Error_InvalidNodeConfig exception if the config fails to verify.
BaseObject? verification = DaqUtils.Call(node, "Setup.Configure.Verify");

if (!(verification?.Cast<PropertyObject>().GetPropertyValue("Success") ?? false))
{
    Console.WriteLine("\nFailed to verify the configuration. The following issues were found: \n");
    // Print out all the issues that were found
    Console.WriteLine($"{verification?.Cast<PropertyObject>().GetPropertyValue("Issues")} \n");
    Console.WriteLine("Configuration will not be applied. \n");
}
else
{
    // Apply the configuration to the Node
    // Note: This writes multiple options to the Node. If an Error_NodeCommunication
    //       exception is thrown, it is possible that some options were successfully
    //       applied, while others failed. It is recommended to keep calling
    //       Apply until no exception is thrown.
    BaseObject? application = DaqUtils.Call(node, "Setup.Configure.Apply");
    if (!(application?.Cast<PropertyObject>().GetPropertyValue("Success") ?? false))
    {
        // Print out all the issues that were found
        Console.WriteLine($"{application?.Cast<PropertyObject>().GetPropertyValue("Issues")} \n");
        Console.WriteLine("Application failed. \n");
    }
}

Console.WriteLine("Done. \n");
```

### Starting Sync Sampling

Synchronized Sampling is a sampling mode that automatically coordinates all incoming node data to a particular gateway. It is designed to ensure data arrival and sequence.

This code snippet provides the function to start sync sampling:

```c#
// Select nodes
foreach (Channel node in base_station.GetChannels())
{
    Console.WriteLine($"Adding node {node.Cast<PropertyObject>().GetPropertyValue("Advanced.NodeAddress")} \n");

    // Enable the node for sampling
    node.Cast<PropertyObject>().SetPropertyValue("Control.Sample.Enabled", true);

    // Add the node to the network
    BaseObject? add_result = DaqUtils.Call(base_station, "Control.Sample.AddNode", node.Cast<PropertyObject>().GetPropertyValue("Advanced.NodeAddress")!.Cast<IntegerObject>());
    if (!(add_result?.Cast<PropertyObject>().GetPropertyValue("Success") ?? false))
    {
        Console.WriteLine($"Adding node failed: {add_result?.Cast<PropertyObject>().GetPropertyValue("Error")} \n");
    }
}

// Can get information about the network
Console.WriteLine("Network info: \n");
Console.WriteLine($"Network OK: {base_station.Cast<PropertyObject>().GetPropertyValue("Control.Sample.IsValid")} \n");
Console.WriteLine($"Percent of Bandwidth: {base_station.Cast<PropertyObject>().GetPropertyValue("Control.Sample.NetworkBandwidth")} \n");
Console.WriteLine($"Lossless Enabled: {base_station.Cast<PropertyObject>().GetPropertyValue("Control.Sample.Lossless")} \n");

// Start the network once all nodes have been added
BaseObject? start_result = DaqUtils.Call(base_station, "Control.Sample.ApplyAndStartNetwork");
Console.WriteLine($"Start network succeeded: {start_result?.Cast<PropertyObject>().GetPropertyValue("Success") ?? false} \n");
```

> Note: The Nodes must already be configured for Sync Sampling before adding to the network, or else Error_InvalidNodeConfig will be thrown.
>
> If you wish to provide your own start time (instead of using the system time), pass a Timestamp object as a second parameter to this function.
>
> If you don't want to enable a beacon at this time, use the `Control.Sample.ApplyAndArmNodes` function. The nodes will wait until they hear a beacon to start sampling.

### Enabling Beacons

The beacon is used to synchronize and start a group of nodes when performing Synchronized Sampling.

To enable a beacon:

```c#
// Make sure we can ping the base station
PropertyObject? enableBeaconResult = DaqUtils.Call(base_station, "Control.EnableBeacon")?.Cast<PropertyObject>();
BaseObject? beaconTime = enableBeaconResult?.GetPropertyValue("Timestamp");

if (!(enableBeaconResult?.GetPropertyValue("Success") ?? false))
{
    Console.WriteLine("Failed to ping the Base Station \n");
}

if (base_station.Cast<PropertyObject>().GetPropertyValue("Capabilities.SupportsBeaconStatus"))
{
    BaseObject? status = DaqUtils.Call(base_station, "Control.GetBeaconStatus");
    Console.WriteLine($"Beacon current status: Enabled?: {(status?.Cast<PropertyObject>().GetPropertyValue("Enabled") ?? false ? "TRUE" : "FALSE")}");
    Console.WriteLine($"Time: {status?.Cast<PropertyObject>().GetPropertyValue("Timestamp")} \n");
}

Console.WriteLine("Attempting to enable the beacon... \n");

// Enable the beacon on the Base Station using the PC time
if (!(enableBeaconResult?.GetPropertyValue("Success") ?? false))
{
    Console.WriteLine("Failed to enable the beacon \n");
}
else
{
    // If we got here, no exception was thrown, so enableBeacon was successful
    Console.WriteLine("Successfully enabled the beacon on the Base Station \n");
    Console.WriteLine($"Beacon's initial Timestamp: {beaconTime} \n");
    Console.WriteLine("Beacon is active \n");
}
```

### Disabling Beacons

To disable a beacon:

```c#
// Disable the beacon on the Base Station
PropertyObject? disableBeaconResult = DaqUtils.Call(base_station, "Control.DisableBeacon")?.Cast<PropertyObject>();

if (!(disableBeaconResult?.GetPropertyValue("Success") ?? false))
{
    Console.WriteLine("Failed to disable the beacon \n");
}
else
{
    // If we got here, disableBeacon was successful
    Console.WriteLine("Successfully disabled the beacon on the Base Station \n");
}
```

### Streaming Data

The openDAQ module exposes each physical measurement channel as a separate signal. Data is read using a stream reader, which buffers incoming samples and lets you consume them at your own pace.

```c#
using System.Collections.Generic;
using System.Threading;

// openDAQ delivers each measurement channel through its own signal, so a
// separate StreamReader is needed for each one. Readers must be created once
// before the read loop. Creating them inside the loop would discard any samples
// buffered since the previous iteration.
//
// Signals only exist after the node has sent its first packet of data, so this
// list will be empty if sampling has not started yet. Make sure the network is
// running before building it.
List<(string NodeAddress, string ChannelName, Daq.Core.OpenDAQ.StreamReader<double, ulong> Reader)> nodeSignalReaders = new List<(string NodeAddress, string ChannelName, Daq.Core.OpenDAQ.StreamReader<double, ulong> Reader)>();

while (nodeSignalReaders.Count == 0)
{
    foreach (Channel n in base_station.GetChannels())
    {
        string nodeAddress = n.Cast<PropertyObject>()
            .GetPropertyValue("Advanced.NodeAddress")
            .ToString();

        foreach (Signal signal in n.GetSignals(SearchFactory.Recursive(SearchFactory.Any())))
        {
            if (signal.DomainSignal is null) continue;

            Daq.Core.OpenDAQ.StreamReader<double, ulong> reader = 
                OpenDAQFactory.CreateStreamReader<double, ulong>(
                    signal, 
                    ReadMode.Scaled, 
                    ReadTimeoutType.Any
            );

            nodeSignalReaders.Add((nodeAddress, signal.Name, reader));
        }
    }
}

// Poll for new samples
while (true)
{
    foreach ((string NodeAddress, string ChannelName, Daq.Core.OpenDAQ.StreamReader<double, ulong> Reader) item in nodeSignalReaders)
    {
        nuint count = 1; // request at least one sample
        double[] values = new double[1];
        ulong[] timestamps = new ulong[1];

        item.Reader.ReadWithDomain(values, timestamps, ref count, 1000);

        if (count > 0)
            Console.WriteLine($"Node {item.NodeAddress} | {item.ChannelName}: {values[0]} @ {timestamps[0]}");
    }

    Thread.Sleep(10);

}
```

> **Note:** Each measurement channel is delivered independently through its own signal. Samples taken at the same time across multiple channels on a node will share the same timestamp value.

### Downloading Logged Data

A Node can keep recording to its own internal memory even if it loses connection to the Base Station. That data can be downloaded into openDAQ signals just like live data.

To view the current datalogging status for a node:

```c#
Console.WriteLine($"Datalog sessions stored on node: {node.Cast<PropertyObject>().GetPropertyValue("Setup.DataManagement.NumDatalogSessions")} \n");
Console.WriteLine($"Node storage full: {node.Cast<PropertyObject>().GetPropertyValue("Setup.DataManagement.PercentFull")} \n");
```

Before starting a download, call `PrepareDownload` to pre-create the datalog signals so readers can be attached to them.

> Note: If no reader is attached before `DownloadData` is called, the download will fail with an error.

```c#
BaseObject? prepare_result = DaqUtils.Call(node, "Setup.DataManagement.PrepareDownload");

if (!prepare_result?.Cast<PropertyObject>().GetPropertyValue("Success"))
{
    Console.WriteLine("Failed to prepare for download \n");
}
```

Each logged channel is exposed as its own signal, named with a "DataLog_ch" prefix. Once the download is prepared, attach readers to each:

```c#
List<(string ChannelName, Daq.Core.OpenDAQ.StreamReader<double, ulong> Reader)> datalogReaders = new List<(string ChannelName, Daq.Core.OpenDAQ.StreamReader<double, ulong> Reader)>();

foreach (Signal signal in node.GetSignals(SearchFactory.Recursive(SearchFactory.Any())))
{
    string name = signal.Name;
    if (!name.StartsWith("DataLog_ch", StringComparison.Ordinal))
        continue;

    Daq.Core.OpenDAQ.StreamReader<double, ulong> reader = OpenDAQFactory.CreateStreamReader<double, ulong>(
        signal, ReadMode.Scaled, ReadTimeoutType.Any);

    datalogReaders.Add((name, reader));
}
```

With readers in place, start the download:

```c#
// The argument is the number of retries allowed per sweep before the download is aborted
BaseObject? download_result = DaqUtils.Call(node, "Setup.DataManagement.DownloadData", (IntegerObject)3);

if (!(download_result?.Cast<PropertyObject>().GetPropertyValue("Success") ?? false))
{
    Console.WriteLine("Failed to start download \n");
}
else
{
    Console.WriteLine("Download started \n");
}
```

The download runs asynchronously, so `DownloadData` returns immediately and progress must be polled separately:

```c#
// Poll for progress until the download reaches a terminal state
while (true)
{
    BaseObject? progress = DaqUtils.Call(node, "Setup.DataManagement.DownloadProgress");
    string state = progress?.Cast<PropertyObject>().GetPropertyValue("State") ?? "Unknown";
    Console.WriteLine($"State: {state}, Progress: {progress?.Cast<PropertyObject>().GetPropertyValue("Progress") ?? 0}% \n");

    if (state == "Complete" || state == "Failed" || state == "Canceled")
        break;

    // Read out any samples that have arrived so far
    foreach ((string ChannelName, Daq.Core.OpenDAQ.StreamReader<double, ulong> Reader) item in datalogReaders)
    {
        nuint count = item.Reader.AvailableCount;

        if (count > 0)
        {
            int len = checked((int)count);
            double[] values = new double[len];
            ulong[] timestamps = new ulong[len];
            item.Reader.ReadWithDomain(values, timestamps, ref count);
            
            for (nuint i = 0; i < count; ++i)
            {
                Console.WriteLine($"{item.ChannelName}: {values[i]} @ {timestamps[i]} \n");
            }
        }
    }

    Thread.Sleep(100);
}
```

To stop a download before it finishes, call `CancelDownload`:

```c#
BaseObject? cancel_result = DaqUtils.Call(node, "Setup.DataManagement.CancelDownload");

if (!(cancel_result?.Cast<PropertyObject>().GetPropertyValue("Success") ?? false))
{
    Console.WriteLine("Failed to cancel download \n");
}
```

> **Note:** A download can only be canceled while it is in progress. Calling `CancelDownload` when no download is running returns `Success` as False.