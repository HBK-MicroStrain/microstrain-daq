## Wireless Python Examples

This document provides examples of common operations with wireless devices using the Python API.

### Pinging Nodes

The ping command is used to check if there is proper communication between the base station and the node.

```python
result = daq_utils.call(node, "Control.Ping")

if result.get_property_value('Success'):

    # Get some details from the response
    print("Successfully pinged Node {0}".format(node.get_property_value('Advanced.NodeAddress')))
    print("Base Station RSSI: {0}".format(result.get_property_value('BaseRssi')))
    print("Node RSSI: {0}".format(result.get_property_value('NodeRssi')))

    # We can talk to the Node, so let's get some more info
    print("\nNode Information:")
    print("--------------------")
    print("Model Number: {0}".format(node.get_property_value('Advanced.ModelName')))
    print("Serial: {0}".format(node.get_property_value('Advanced.Serial')))
    print("Firmware: {0}\n".format(node.get_property_value('Advanced.FirmwareVersion')))
else:
    print("Failed to ping Node {0}".format(node.get_property_value('Advanced.NodeAddress')))
```
> **Note: To communicate with a Wireless Node, all the following must be true:**
>
>    - The Node is powered on, and within range of the Base Station
>    - The Node is on the same frequency as the Base Station
>    - The Node is in Idle Mode (not sampling, and not sleeping)
>    - The Node is on the same communication protocol as the Base Station (LXRS vs LXRS+)

### Setting to Idle

The **Set to Idle** command is used to put a Node that is sampling, or sleeping, back into the Idle Mode so that it may be communicated with.

```python
# The integer argument here is the command timeout in milliseconds
result = daq_utils.call(node, "Control.SetToIdle", 3000)

print("Setting Node to Idle")

# Using the SetToIdleResult object, check if the Set to Idle operation is complete.
# NUM_CHECKS is used here as an arbitrary limit while checking the idle status
for i in range(NUM_CHECKS):
    success = result.get_property_value('Success')                                        
    status = result.get_property_value('Status')                                          
    print(f"Status: [Success={success}, Status={status}]") 
                          
    if success:   
        break

# Check the result of the Set to Idle operation
final_status = result.get_property_value('Status')

# Completed successfully
if final_status == daq_types.enum("SetToIdleResult", "setToIdleResult_success"):
    print("Successfully set to idle!")
# Canceled by the user
elif final_status == daq_types.enum("SetToIdleResult", "setToIdleResult_canceled"):
    # Canceled by the user
    print("Set to Idle was canceled!")
# Failed to perform the operation
else:
    print("Set to Idle has failed!")
```

### Getting Current Configuration Settings

Configuration indicates how nodes are set up for data acquisition. It includes settings such as sampling mode/rate, offsets, hardware gain, etc.

To read current configuration settings on the node:

```python
print("Current Configuration Settings")

# Read some of the current configuration settings on the node
print("# of Triggers: {0}".format(node.get_property_value('Setup.DataManagement.NumDatalogSessions')))
print("User Inactivity Timeout: {0} seconds".format(node.get_property_value('Setup.Configure.Power.InactivityTimeout')))
print("Total active channels: {0}".format(node.get_property_value('Control.Sample.ActiveChannels')))
print("# of sweeps: {0}".format(node.get_property_value('Control.Sample.NumSweeps')))
```

If a configuration function requires a channel mask parameter, this indicates that the option may affect one or more channels on the Node. You can either: 

- Provide the channel mask when asking for the configuration (if known beforehand)
- Programmatically determine the mask for each setting

#### Programmatically Determining The Mask For Each Setting

```python
chGroups = daq_utils.call(node, "Capabilities.ChannelGroups")

for groups in chGroups.get_property_value('Result'):

    for settings in groups.Settings:
        # If the group contains the linear equation setting
        if settings == daq_types.enum("ChannelGroupSetting", "chSetting_linearEquation"):
            # We can now pass the channel mask for this group to the function.
            # Note: once this channel mask is known for a specific node (+ fw version), it should never change
            eq = daq_utils.call(node, "Setup.Configure.Calibration.GetLinearEquation", groups.Mask)

            print("Linear Equation for: {0}".format(groups.Name))
            print("Slope: {0:06.3f}".format(le.get_property_value("Slope")))
            print("Offset: {0:06.3f}".format(le.get_property_value("Offset")))
```

### Setting Current Configuration Settings

To set current configuration settings for a node:

```python
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
    application = daq_utils.call(node, "Setup.Configure.Apply")
    if not application.get_property_value("Success"):
        # Print out all the issues that were found
        print(application.get_property_value("Issues"))

        print("Application failed.")

print("Done.")
```

### Starting Sync Sampling

Synchronized Sampling is a sampling mode that automatically coordinates all incoming node data to a particular gateway. It is designed to ensure data arrival and sequence.

This code snippet provides the function to start sync sampling:

```python
# Select nodes
for node in base_station.get_channels():
    print(f"Adding node {node.get_property_value('Advanced.NodeAddress')}")

    # Enable the node for sampling
    node.set_property_value('Control.Sample.Enabled', True)

    # Add the node to the network
    add_result = daq_utils.call(base_station, 'Control.Sample.AddNode', node.get_property_value('Advanced.NodeAddress'))
    if not add_result.get_property_value('Success'):
        print(f"Adding node failed: {add_result.get_property_value('Error')}")

# Can get information about the network
print("Network info:")
print("Network OK: {0}".format(base_station.get_property_value("Control.Sample.IsValid")))
print(f"Percent of Bandwidth: {(base_station.get_property_value("Control.Sample.NetworkBandwidth"))}%")
print("Lossless Enabled: {0}".format(base_station.get_property_value("Control.Sample.Lossless")))

# Start the network once all nodes have been added
start_result = daq_utils.call(base_station, 'Control.Sample.ApplyAndStartNetwork')
print(f"Start network succeeded: {start_result.get_property_value('Success')}")
```

> Note: The Nodes must already be configured for Sync Sampling before adding to the network, or else Error_InvalidNodeConfig will be thrown.
>
> If you wish to provide your own start time (instead of using the system time), pass a Timestamp object as a second parameter to this function.
>
> If you don't want to enable a beacon at this time, use the `Control.Sample.ApplyAndArmNodes` function. The nodes will wait until they hear a beacon to start sampling.

### Enabling Beacons

The beacon is used to synchronize and start a group of nodes when performing Synchronized Sampling.

To enable a beacon:

```python
# Make sure we can ping the base station
if not (daq_utils.call(base_station, "Control.Ping")).get_property_value("Success"):
    print("Failed to ping the Base Station")

if base_station.get_property_value("Capabilities.SupportsBeaconStatus"):
    status = daq_utils.call(base_station, "Control.GetBeaconStatus")
    print("Beacon current status: Enabled?: {0}".format("TRUE" if status.get_property_value("Enabled") else "FALSE"), end="")
    print(" Time: {0}".format(status.get_property_value("Timestamp")))

print("Attempting to enable the beacon...")

# Enable the beacon on the Base Station using the PC time
beaconTime = (daq_utils.call(base_station, "Control.EnableBeacon")).get_property_value("Timestamp")
if not (daq_utils.call(base_station, "Control.EnableBeacon")).get_property_value("Success"):
    print("Failed to enable the beacon")
else:
    # If we got here, no exception was thrown, so enableBeacon was successful
    print("Successfully enabled the beacon on the Base Station")
    print("Beacon's initial Timestamp: {0}".format(beaconTime))

    print("Beacon is active")
```

### Disabling Beacons

To disable a beacon:

```python
# Disable the beacon on the Base Station
daq_utils.call(base_station, "Control.DisableBeacon")

if not (daq_utils.call(base_station, "Control.DisableBeacon")).get_property_value("Success"):
    print("Failed to disable the beacon")
else:
    # If we got here, disableBeacon was successful
    print("Successfully disabled the beacon on the Base Station")
```