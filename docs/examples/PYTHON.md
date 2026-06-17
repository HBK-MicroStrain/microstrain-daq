## Usage

### Pinging Nodes

This code snippet provides the function to ping nodes:

```
def pingNode(node):
    response = node.ping()

    if response.success():
    
        # Get some details from the response
        print("Successfully pinged Node {0}".format(node.nodeAddress()))
        print("Base Station RSSI: {0}".format(response.baseRssi()))
        print("Node RSSI: {0}".format(response.nodeRssi()))

        # We can talk to the Node, so let's get some more info
        print("Node Information:")
        print("Model Number: {0}".format(node.model()))
        print("Serial: {0}".format(node.serial()))
        print("Firmware: {0}\n".format(node.firmwareVersion()))
    else:
        print("Failed to ping Node {0}".format(node.nodeAddress()))
```
> **Note: To communicate with a Wireless Node, all the following must be true:**
>
>    - The Node is powered on, and within range of the BaseStation
>    - The Node is on the same frequency as the BaseStation
>    - The Node is in Idle Mode (not sampling, and not sleeping)
>    - The Node is on the same communication protocol as the BaseStation (LXRS vs LXRS+)

### Getting Current Configuration Settings

To read current configuration settings on the node:

```
def getCurrentConfig(node):
    print("Current Configuration Settings")

    # Read some of the current configuration settings on the node
    print("# of Triggers: {0}".format(node.getNumDatalogSessions()))
    print("User Inactivity Timeout: {0} seconds".format(node.getInactivityTimeout()))
    print("Total active channels: {0}".format(node.getActiveChannels().count()))
    print("# of sweeps: {0}".format(node.getNumSweeps()))
```

If a configuration function requires a ChannelMask parameter, this indicates that the option may affect 1 or more channels on the Node. For instance, a hardware gain may affect ch1 and ch2 with just 1 setting. If you know the mask for your Node, you can provide that mask when asking for the configuration. If you want to programmatically determine the mask for each setting, you can ask for the Node's ChannelGroups. Add the following code to the previous snippet.

```
    # Get the ChannelGroups that the node supports
    chGroups = node.features().channelGroups()

    # Iterate over each channel group
    for group in chGroups:
        # Get all the settings for this group (i.e., may contain linear equation and hardware gain).
        groupSettings = group.settings()

        # Iterate over each setting for this group
        for setting in groupSettings:
            # If the group contains the linear equation setting
            if setting == mscl.WirelessTypes.chSetting_linearEquation:
                # We can now pass the channel mask (group.channels()) for this group to the node.getLinearEquation function.
                # Note: once this channel mask is known for a specific node (+ fw version), it should never change
                le = node.getLinearEquation(group.channels())

                print("Linear Equation for: {0}".format(group.name()))
                print("Slope: {0:06.3f}".format(le.slope()))
                print("Offset: {0:06.3f}".format(le.offset()))
```

### Starting Sync Sampling

This code snippet provides the function to start sync sampling:

> Note: The Nodes must already be configured for Sync Sampling before adding to the network, or else Error_InvalidNodeConfig will be thrown.

```
def startSyncSampling(baseStation, nodes):
    # Create a SyncSamplingNetwork object, giving it the BaseStation that will be the master BaseStation for the network.
    network = mscl.SyncSamplingNetwork(baseStation)

    # Add the WirelessNodes to the network.
    for node in nodes:
        print("Adding node {0} to the network...".format(node.nodeAddress()), end="")
        network.addNode(node)
        print("Done.")

    # Can get information about the network
    print("Network info:")
    print("Network OK: {0}".format("TRUE" if network.ok() else "FALSE"))
    print("Percent of Bandwidth: {0:04.02f}%", network.percentBandwidth())
    print("Lossless Enabled: {0}".format("TRUE" if network.lossless() else "FALSE"))

    # Apply the network configuration to every node in the network
    print("Applying network configuration...", end="")
    network.applyConfiguration()
    print("Done.")

    # Start all the nodes in the network sampling. The master BaseStation's beacon will be enabled with the system time.
    print("Starting the network...", end="")
    network.startSampling()
    print("Done.")
```

> Note: If you wish to provide your own start time (not use the system time), pass a mscl::Timestamp object as a second parameter to this function.

> Note: If you do not want to enable a beacon at this time, use the startSampling_noBeacon() function. (The nodes will wait until they hear a beacon to start sampling).

Many other functions are available for the SyncSamplingNetwork:

| Function | Description |
|----------|-------------|
| network.lossless() | Enable or disable "lossless" mode for the network (default of enabled). |
| network.ok() | Check whether the network is "OK" meaning all nodes fit in the network and have communicated successfully. |
| network.percentBandwidth() | Get the percentage of bandwidth for the entire network. |
|network.refresh() | Refreshes the entire network. Should be called any time a change is made to the node after it has been added to the network. |
|network.removeNode() | Remove a node from the network. |
| network.getNodeNetworkInfo(nodeAddress) | Get network information for an individual node in the network (TDMA address, percent bandwidth, etc.) |

### Enabling Beacons

To enable a beacon:

```
# Make sure we can ping the base station
if not baseStation.ping():
    print("Failed to ping the Base Station")

if baseStation.features().supportsBeaconStatus():
    status = baseStation.beaconStatus()
    print("Beacon current status: Enabled?: {0}".format("TRUE" if status.enabled() else "FALSE"), end="")
    print(" Time: {0}".format(status.timestamp()))

print("Attempting to enable the beacon...")

# Enable the beacon on the Base Station using the PC time
beaconTime = baseStation.enableBeacon()

# If we got here, no exception was thrown, so enableBeacon was successful
print("Successfully enabled the beacon on the Base Station")
print("Beacon's initial Timestamp: {0}".format(beaconTime))

print("Beacon is active")
```

### Disabling Beacons

To disable a beacon:

```
# Disable the beacon on the Base Station
baseStation.disableBeacon()

# If we got here, no exception was thrown, so disableBeacon was successful
print("Successfully disabled the beacon on the Base Station")
```