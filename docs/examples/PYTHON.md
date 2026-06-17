## Usage

### Pinging Nodes

This code snippet provides the function to ping nodes.

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