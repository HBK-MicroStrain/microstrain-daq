# MicroStrain DAQ

![platform](https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-informational) ![Apache 2.0](https://img.shields.io/badge/License-Apache%202.0-blue) [![Latest](https://img.shields.io/github/v/release/HBK-MicroStrain/microstrain-daq?include_prereleases&label=Latest)](https://github.com/HBK-MicroStrain/microstrain-daq/releases/latest)

[![C++](https://img.shields.io/badge/C%2B%2B-informational?logo=github)](https://github.com/HBK-MicroStrain/microstrain-daq) [![C#](https://img.shields.io/nuget/dt/MicroStrain.DaqUtils?logo=nuget&label=C%23)](https://www.nuget.org/packages/MicroStrain.DaqUtils) [![Python (daq-utils)](https://img.shields.io/pypi/dm/microstrain-daq-utils?logo=pypi&label=Python%20%28daq-utils%29)](https://pypi.org/project/microstrain-daq-utils/) [![Python (jupyter)](https://img.shields.io/pypi/dm/microstrain-daq-jupyter?logo=pypi&label=Python%20%28jupyter%29)](https://pypi.org/project/microstrain-daq-jupyter/)

[![CI C++ (Windows)](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/ci-cpp-windows.yml/badge.svg)](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/ci-cpp-windows.yml) [![CI C++ (Linux)](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/ci-cpp-linux.yml/badge.svg)](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/ci-cpp-linux.yml) [![CI C#](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/ci-csharp.yml/badge.svg)](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/ci-csharp.yml) [![CD](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/cd.yml/badge.svg)](https://github.com/HBK-MicroStrain/microstrain-daq/actions/workflows/cd.yml)

An opinionated library that enhances working with [openDAQ](https://opendaq.com/) modules from [MicroStrain by HBK](https://www.hbkworld.com/en/Campaign/microstrain-by-hbk).

## Installation

### C++

Add the library to your project using CMake FetchContent, replacing `<version>` with the desired release tag:

```cmake
include(FetchContent)
FetchContent_Declare(
    MicroStrainDaqUtils
    GIT_REPOSITORY https://github.com/HBK-MicroStrain/microstrain-daq.git
    GIT_TAG        <version>
    SOURCE_SUBDIR  microstrain-daq-utils/daq_utils/cpp
)
FetchContent_MakeAvailable(MicroStrainDaqUtils)

target_link_libraries(your_target PRIVATE microstrain::daq_utils)
```

To import the library into your project:

```cpp
#include <daq_utils/daq_utils.h>
#include <daq_utils/wireless.h>
```

### Python

```
pip install microstrain-daq-utils
```

To import the library into your project:

```python
import daq_utils
```

### C#

```
dotnet add package MicroStrain.DaqUtils
```

To import the library into your project:

```csharp
using Daq.Utils;
```

See [Usage](#usage) for examples of how to use the library.

## JupyterLab

The Python library bindings can be used with [JupyterLab](https://jupyterlab.readthedocs.io/en/stable/), and a separate package provides notebook templates pre-configured for various use cases.

This is ideal for exploration, prototyping, and testing.

### Installation

```
pip install microstrain-daq-jupyter
```

> **Note:** Notebook templates require the [Library](#library) to be installed.

### Running a Session

Navigate to the directory where you would like to save your notebooks and launch JupyterLab:

```
jupyter lab
```

### Creating New Notebooks

To create a new notebook from a template, click the new launcher button:

![New launcher button in JupyterLab](docs/images/new-launcher-button.png)

Then, click the `Template` tile in the `Notebook` section:

![Template button in JupyterLab instance launcher](docs/images/template-tile.png)

Then, select the desired template:

![Template selector in JupyterLab instance launcher](docs/images/template-selector.png)

#### Available Templates

| Template | Description |
|----------|-------------|
| Starter  | Pre-configured openDAQ and library setup ready to use |

## Usage

See the openDAQ [documentation](https://docs.opendaq.com/manual/opendaq/3.30/introduction.html) for a full reference on the openDAQ API. For wireless-specific usage and examples, see the [Wireless guide](docs/WIRELESS.md).

### Adding modules

By default, openDAQ loads modules from its installation directory. To load modules from a different location, set the `OPENDAQ_MODULE_PATH` environment variable to the desired directory.

For example, to set it to the `Downloads` directory:

**Windows**
```
setx OPENDAQ_MODULE_PATH C:\Users\username\Downloads
```

> **Note:** `setx` takes effect in new terminal sessions, not the current one. Restart your terminal after running this command.

**Linux**
```
touch ~/.bashrc && \
sed -i '/^export OPENDAQ_MODULE_PATH=/d' ~/.bashrc && \
echo 'export OPENDAQ_MODULE_PATH=~/Downloads' >> ~/.bashrc && \
source ~/.bashrc
```

**Mac**
```
touch ~/.bashrc && \
sed -i '' '/^export OPENDAQ_MODULE_PATH=/d' ~/.bashrc && \
echo 'export OPENDAQ_MODULE_PATH=~/Downloads' >> ~/.bashrc && \
source ~/.bashrc
```

> **Note:** If multiple versions of the same module exist in the directory, the behavior is undefined. Remove the old version before adding the new one.

Then pass the path when creating your openDAQ instance:

**C++**
```cpp
#include <opendaq/opendaq.h>

daq::InstanceBuilderPtr builder = daq::InstanceBuilder();
if (const char* modulePath = std::getenv("OPENDAQ_MODULE_PATH")) {
    builder.setModulePath(modulePath);
}
daq::InstancePtr instance = builder.build();
```

**Python**
```python
import os
import opendaq as daq

builder = daq.InstanceBuilder()
if module_path := os.environ.get('OPENDAQ_MODULE_PATH'):
    builder.module_path = module_path
instance = builder.build()
```

**C#**
```csharp
using Daq.Core.OpenDAQ;

var builder = OpenDAQFactory.InstanceBuilder();
var modulePath = Environment.GetEnvironmentVariable("OPENDAQ_MODULE_PATH");
if (modulePath != null)
    builder.ModulePath = modulePath;
var instance = builder.Build();
```

### Discovering devices

This code snippet will display a list of all currently available devices:

**C++**
```cpp
for (daq::DeviceInfoPtr deviceInfo : instance.getAvailableDevices()) {
    std::cout << "Name: " << deviceInfo.getName() << " Connection string: " << deviceInfo.getConnectionString() << "\n";
}
```

**Python**
```python
for device_info in instance.available_devices:
    print('Name:', device_info.name, 'Connection string:', device_info.connection_string)
```

**C#**
```csharp
foreach (var deviceInfo in instance.AvailableDevices)
    Console.WriteLine($"Name: {deviceInfo.Name} Connection string: {deviceInfo.ConnectionString}");
```

### Adding devices

Add a device using its connection string:

**C++**
```cpp
daq::DevicePtr device = instance.addDevice("microstrain-wireless://COM46:3000000");
```

**Python**
```python
device = instance.add_device('microstrain-wireless://COM46:3000000')
```

**C#**
```csharp
var device = instance.AddDevice("microstrain-wireless://COM46:3000000");
```

Connection strings are in the format: `prefix://address`.

### Removing devices

When you are ready to remove the device:

**C++**
```cpp
instance.removeDevice(device);
```

**Python**
```python
instance.remove_device(device)
```

**C#**
```csharp
instance.RemoveDevice(device);
```

This will disconnect the device so you can use it in other applications.

### Getting channels

Get a reference to a channel using it's index:

**C++**
```cpp
daq::ChannelPtr channel = device.getChannels()[0];
```

**Python**
```python
channel = device.get_channels()[0]
```

**C#**
```csharp
var channel = device.GetChannels()[0];
```

### Getting property groups

Properties are organized into `groups`. To print available property groups for a device, channel, group, or other root:

**C++**
```cpp
daq_utils::PrintGroups(channel);
```

**Python**
```python
daq_utils.print_groups(channel)
```

**C#**
```csharp
DaqUtils.PrintGroups(channel);
```

To get the groups as a list instead:

**C++**
```cpp
std::vector<std::string> groups = daq_utils::Groups(channel);
```

**Python**
```python
daq_utils.groups(channel)
```

**C#**
```csharp
DaqUtils.Groups(channel);
```

### Getting properties

To print all properties across every group:

**C++**
```cpp
daq_utils::PrintProperties(channel);
```

**Python**
```python
daq_utils.print_properties(channel)
```

**C#**
```csharp
DaqUtils.PrintProperties(channel);
```

To filter to a specific group:

**C++**
```cpp
daq_utils::PrintProperties(channel, "Setup.Configure.Sampling");
```

**Python**
```python
daq_utils.print_properties(channel, 'Setup.Configure.Sampling')
```

**C#**
```csharp
DaqUtils.PrintProperties(channel, "Setup.Configure.Sampling");
```

To get the properties as a list instead:

**C++**
```cpp
std::vector<daq_utils::PropertyInfo> props = daq_utils::Properties(channel);
std::vector<daq_utils::PropertyInfo> props = daq_utils::Properties(channel, "Setup.Configure.Sampling");
```

**Python**
```python
daq_utils.properties(channel)
daq_utils.properties(channel, 'Setup.Configure.Sampling')
```

**C#**
```csharp
DaqUtils.Properties(channel);
DaqUtils.Properties(channel, "Setup.Configure.Sampling");
```

### Finding a property path

If you know a property name but not its full path, use `find` to get its full dot-notation path:

**C++**
```cpp
std::string path = daq_utils::Find(channel, "LostBeaconTimeout");
```

**Python**
```python
daq_utils.find(channel, 'LostBeaconTimeout')
```

**C#**
```csharp
DaqUtils.Find(channel, "LostBeaconTimeout");
```

This can also be used for finding groups:

**C++**
```cpp
std::string path = daq_utils::Find(channel, "Sampling");
```

**Python**
```python
daq_utils.find(channel, 'Sampling')
```

**C#**
```csharp
DaqUtils.Find(channel, "Sampling");
```

### Accessing properties

Properties can be accessed using dot-notation paths:

**C++**
```cpp
daq::BaseObjectPtr timeout = channel.getPropertyValue("Setup.Configure.Sampling.LostBeaconTimeout");
```

**Python**
```python
timeout = channel.get_property_value('Setup.Configure.Sampling.LostBeaconTimeout')
```

**C#**
```csharp
var timeout = channel.GetPropertyValue("Setup.Configure.Sampling.LostBeaconTimeout");
```

They can also be set:

**C++**
```cpp
channel.setPropertyValue("Setup.Configure.Sampling.LostBeaconTimeout", 7);
```

**Python**
```python
channel.set_property_value('Setup.Configure.Sampling.LostBeaconTimeout', 7)
```

**C#**
```csharp
channel.SetPropertyValue("Setup.Configure.Sampling.LostBeaconTimeout", (IntegerObject)7);
```

### Inspecting function properties

To view a function property's description, arguments, and return type, read the `description` field from the property:

**C++**
```cpp
std::cout << channel.getProperty("Capabilities.MaxSweeps").getDescription() << "\n";
```

**Python**
```python
print(channel.get_property('Capabilities.MaxSweeps').description)
```

**C#**
```csharp
Console.WriteLine(channel.GetProperty("Capabilities.MaxSweeps").Description);
```

### Calling function properties

Function properties can be called directly through the openDAQ API, but the wrapper simplifies the syntax. To call a function with no arguments using the wrapper:

**C++**
```cpp
daq::BaseObjectPtr result = daq_utils::Call(channel, "Setup.Configure.Apply");
```

**Python**
```python
result = daq_utils.call(channel, "Setup.Configure.Apply")
```

**C#**
```csharp
var result = DaqUtils.Call(channel, "Setup.Configure.Apply");
```

To call a function with arguments:

**C++**
```cpp
daq::BaseObjectPtr result = daq_utils::Call(channel, "Capabilities.InputRangesWithVoltage", {0xFF, 5000});
```

**Python**
```python
result = daq_utils.call(channel, "Capabilities.InputRangesWithVoltage", 0xFF, 5000)
```

**C#**
```csharp
var result = DaqUtils.Call(channel, "Capabilities.InputRangesWithVoltage", (IntegerObject)0xFF, (IntegerObject)5000);
```

The result object can then be queried for any returned properties. For example:

**C++**
```cpp
daq::BaseObjectPtr success = result.asPtr<daq::IPropertyObject>().getPropertyValue("Success");
```

**Python**
```python
success = result.get_property_value('Success')
```

**C#**
```csharp
var success = result?.Cast<PropertyObject>()?.GetPropertyValue("Success");
```

### Inspecting Properties

To view what properties are available for a device, channel, or group, create a `DaqPropertyInspector`:

**C++**
```cpp
daq_utils::DaqPropertyInspector propInspector(instance);
```

**Python**
```python
prop_inspector = daq_utils.DaqPropertyInspector(instance)
```

**C#**
```csharp
var propInspector = new DaqPropertyInspector(instance);
```
To inspect a property:

**C++**
```cpp
propInspector.Describe(node, "Setup.Configure.CommunicationProtocol");
```

**Python**
```python
prop_inspector.describe(node, 'Setup.Configure.CommunicationProtocol')
```

**C#**
```csharp
propInspector.Describe(node, "Setup.Configure.CommunicationProtocol");
```

### Inspecting types

To view what fields/values are available for openDAQ `Enumeration` and `Struct` types, create a `DaqTypeInspector`:

**C++**
```cpp
daq_utils::DaqTypeInspector typeInspector(instance);
```

**Python**
```python
type_inspector = daq_utils.DaqTypeInspector(instance)
```

**C#**
```csharp
var typeInspector = new DaqTypeInspector(instance);
```

To inspect a type:

**C++**
```cpp
typeInspector.Describe("AutoCalCompletionFlag");
```

**Python**
```python
type_inspector.describe('AutoCalCompletionFlag')
```

**C#**
```csharp
typeInspector.Describe("AutoCalCompletionFlag");
```

### Creating typed values

To create openDAQ typed values such as `Enumerations` and `Structs`, use `DaqTypeFactory`. It handles the type manager and string conversion automatically:

**C++**
```cpp
daq_utils::DaqTypeFactory daqTypes(instance);
```

**Python**
```python
daq_types = daq_utils.DaqTypeFactory(instance)
```

**C#**
```csharp
var daqTypes = new DaqTypeFactory(instance);
```

#### Creating an enum value

**C++**
```cpp
daq::EnumerationPtr voltage = daqTypes.MakeEnum("mscl_WirelessTypes_Voltage", "voltage_3000mV");
```

**Python**
```python
voltage = daq_types.enum("mscl_WirelessTypes_Voltage", "voltage_3000mV")
```

**C#**
```csharp
var voltage = daqTypes.MakeEnum("mscl_WirelessTypes_Voltage", "voltage_3000mV");
```

#### Creating a Struct value

**C++**
```cpp
daq::StructPtr cmdInfo = daqTypes.MakeStruct("mscl_ShuntCalCmdInfo",
{
    {"UseInternalShunt",  daq::Boolean(true)},
    {"NumActiveGauges",   daq::Integer(1)},
    {"GaugeResistance",   daq::Integer(350)},
    {"ShuntResistance",   daq::Integer(100000)},
    {"GaugeFactor",       daq::Float(2.0)},
    {"InputRange",        daqTypes.MakeEnum("mscl_WirelessTypes_InputRange", "range_14_545mV")},
    {"HardwareOffset",    daq::Integer(0)},
    {"ExcitationVoltage", daqTypes.MakeEnum("mscl_WirelessTypes_Voltage", "voltage_1500mV")}
});
```

**Python**
```python
cmd_info = daq_types.struct(
    "mscl_ShuntCalCmdInfo",
    {
        "UseInternalShunt": True,
        "NumActiveGauges": 1,
        "GaugeResistance": 350,
        "ShuntResistance": 100000,
        "GaugeFactor": 2.0,
        "InputRange": daq_types.enum("mscl_WirelessTypes_InputRange", "range_14_545mV"),
        "HardwareOffset": 0,
        "ExcitationVoltage": daq_types.enum("mscl_WirelessTypes_Voltage", "voltage_1500mV")
    }
)
```

**C#**
```csharp
var cmdInfo = daqTypes.MakeStruct("mscl_ShuntCalCmdInfo", new Dictionary<string, object>
{
    ["UseInternalShunt"] = true,
    ["NumActiveGauges"] = 1,
    ["GaugeResistance"] = 350,
    ["ShuntResistance"] = 100000,
    ["GaugeFactor"] = 2.0,
    ["InputRange"] = daqTypes.MakeEnum("mscl_WirelessTypes_InputRange", "range_14_545mV"),
    ["HardwareOffset"] = 0,
    ["ExcitationVoltage"] = daqTypes.MakeEnum("mscl_WirelessTypes_Voltage", "voltage_1500mV")
});
```

### Using openDAQ containers

#### Structs

To read a field from a struct by name:

**C++**
```cpp
daq::StructPtr struct_val = result.asPtr<daq::IStruct>();
daq::BaseObjectPtr field = struct_val.get("FieldName");
```

**Python**
```python
field = struct_val.FieldName
```

**C#**
```csharp
var field = struct_val.Cast<Struct>()?.Get("FieldName");
```

To iterate over all fields:

**C++**
```cpp
daq::StructPtr struct_val = result.asPtr<daq::IStruct>();
for (const daq::StringPtr& name : struct_val.getFieldNames()) {
    std::cout << name << ": " << struct_val.get(name) << "\n";
}
```

**Python**
```python
for name in struct_val.struct_type.field_names:
    print(name, getattr(struct_val, name))
```

**C#**
```csharp
var struct_val = result.Cast<Struct>();
foreach (string name in struct_val.FieldNames)
    Console.WriteLine($"{name}: {struct_val.Get(name)}");
```

> **Note:** To see what fields a struct type contains, see [Inspecting types](#inspecting-types).

#### Lists

To access an item by index:

**C++**
```cpp
daq::ListPtr<daq::IBaseObject> list = result.asPtr<daq::IList<daq::IBaseObject>>();
daq::BaseObjectPtr first = list[0];
```

**Python**
```python
first = result[0]
```

**C#**
```csharp
var list = result.Cast<IListObject<BaseObject>>();
var first = list[0];
```

To iterate over all items:

**C++**
```cpp
daq::ListPtr<daq::IBaseObject> list = result.asPtr<daq::IList<daq::IBaseObject>>();
for (const daq::BaseObjectPtr& item : list) {
    std::cout << item << "\n";
}
```

**Python**
```python
for item in result:
    print(item)
```

**C#**
```csharp
var list = result.Cast<IListObject<BaseObject>>();
foreach (var item in list)
    Console.WriteLine(item);
```


## Troubleshooting

### Listing detected modules

If a device isn't being detected, run this to check whether the module is loaded:

#### C++

```cpp
for (const daq::ModulePtr& module : instance.getModuleManager().getModules()) {
    daq::ModuleInfoPtr info = module.getModuleInfo();
    daq::VersionInfoPtr v = info.getVersionInfo();
    std::cout << info.getName() << " (" << info.getId() << ") v" << v.getMajor() << "." << v.getMinor() << "." << v.getPatch() << "\n";
}
```

#### Python

```python
for module in instance.module_manager.modules:
    info = module.module_info
    v = info.version_info
    print(f"{info.name} ({info.id}) v{v.major}.{v.minor}.{v.patch}")
```

#### C#

```csharp
foreach (var module in instance.ModuleManager.Modules)
{
    var info = module.ModuleInfo;
    var v = info.VersionInfo;
    Console.WriteLine($"{info.Name} ({info.Id}) v{v.Major}.{v.Minor}.{v.Patch}");
}
```

- **Module appears** — the module loaded, but something is preventing device detection
- **Module missing** — the module itself is not being loaded
