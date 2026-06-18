## Usage

This document provides examples of common operations with wireless devices.

### Pinging Nodes

A ping is a byte transmitted by the gateway to the node. If the byte is echoed by the node, existing communication is indicated between the node and gateway.

```
result = daq_utils.call(node, "Control.Ping")

if result.get_property_value('Success'):
    
    # Get some details from the response
    print("Successfully pinged Node {0}".format(node.get_property_value('Advanced.NodeAddress')))
    print("Base Station RSSI: {0}".format(result.get_property_value('BaseRssi')))
    print("Node RSSI: {0}".format(result.get_property_value('NodeRssi')))

    # We can talk to the Node, so let's get some more info
    print("Node Information:")
    print("Model Number: {0}".format(node.get_property_value('Advanced.ModelName')))
    print("Serial: {0}".format(node.get_property_value('Advanced.Serial')))
    print("Firmware: {0}\n".format(node.get_property_value('Advanced.FirmwareVersion')))
else:
    print("Failed to ping Node {0}".format(node.get_property_value('Advanced.NodeAddress')))
```
> **Note: To communicate with a Wireless Node, all the following must be true:**
>
>    - The Node is powered on, and within range of the BaseStation
>    - The Node is on the same frequency as the BaseStation
>    - The Node is in Idle Mode (not sampling, and not sleeping)
>    - The Node is on the same communication protocol as the BaseStation (LXRS vs LXRS+)



### Setting Current Configuration Settings

To set current configuration settings for a node:

> Note: This example only changes a small subset of settings. More settings are available. Please reference the documentation for the full list of functions.

```
print("\nChanging configuration settings...", end="")

# Set the configuration options that we want to change
node.set_property_value('Setup.Configure.Power.DefaultMode', daq_types.enum("DefaultMode", "defaultMode_idle"))
node.set_property_value('Setup.Configure.Power.InactivityTimeout', 7200)
node.set_property_value('Control.Sample.SamplingMode', daq_types.enum("SamplingMode", "samplingMode_sync"))
node.set_property_value('Control.Sample.SampleRate', daq_types.enum("WirelessSampleRate", "sampleRate_256Hz"))
node.set_property_value('Control.Sample.UnlimitedDuration', True)

# Attempt to verify the configuration with the Node we want to apply it to
#  Note: This step is not required before applying; however, the apply will throw an
#        Error_InvalidNodeConfig exception if the config fails to verify.
verification = daq_utils.call(node, "Setup.Configure.Verify")

if not verification.get_property_value("Success"):
    print("\nFailed to verify the configuration. The following issues were found:")

    # Print out all the issues that were found
    print(verification.get_property_value("Issues"))

    print("Configuration will not be applied.")
else:
    # Apply the configuration to the Node
    # Note: This writes multiple options to the Node. If an Error_NodeCommunication 
    #       exception is thrown, it is possible that some options were successfully 
    #       applied, while others failed. It is recommended to keep calling 
    #       Apply until no exception is thrown.
    application =  daq_utils.call(node, "Setup.Configure.Apply")
    if not application.get_property_value("Success"):
        # Print out all the issues that were found
        print(application.get_property_value("Issues"))

        print("Application failed.")
print("Done.")
```

### Enabling Beacons

Beacons are used to update each node's real time clock, providing node-to-node synchronization.

To enable a beacon:

```
# Make sure we can ping the base station
if not (daq_utils.call(baseStation, "Control.Ping")).get_property_value("Success"):
    print("Failed to ping the Base Station")

if baseStation.get_property_value("Capabilities.SupportsBeaconStatus"):
    status = daq_utils.call(baseStation, "Control.GetBeaconStatus")
    print("Beacon current status: Enabled?: {0}".format("TRUE" if status.get_property_value("Enabled") else "FALSE"), end="")
    print(" Time: {0}".format(status.get_property_value("Timestamp")))

print("Attempting to enable the beacon...")

# Enable the beacon on the Base Station using the PC time
beaconTime = (daq_utils.call(baseStation, "Control.EnableBeacon")).get_property_value("Timestamp")

# If we got here, no exception was thrown, so enableBeacon was successful
print("Successfully enabled the beacon on the Base Station")
print("Beacon's initial Timestamp: {0}".format(beaconTime))

print("Beacon is active")
```

### Disabling Beacons

To disable a beacon:

```
# Disable the beacon on the Base Station
daq_utils.call(baseStation, "Control.DisableBeacon")

# If we got here, no exception was thrown, so disableBeacon was successful
print("Successfully disabled the beacon on the Base Station")
```