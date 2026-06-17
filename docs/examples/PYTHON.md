## Usage

### Pinging Nodes

This code snippet provides the function to ping nodes. The print statements are used to get details from the responses.

```
def pingNode(node):
    response = node.ping()

    if response.success():
    
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
>