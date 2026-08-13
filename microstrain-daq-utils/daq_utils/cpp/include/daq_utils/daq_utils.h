#pragma once

#include <initializer_list>
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <coreobjects/property_object_ptr.h>
#include <coretypes/enumeration_ptr.h>
#include <coretypes/struct_ptr.h>
#include <coretypes/type_manager_ptr.h>
#include <opendaq/instance_ptr.h>

namespace daq_utils {

struct PropertyInfo {
    std::string group;
    std::string name;
    std::string type;
};

/// @brief Creates openDAQ typed values (enumerations, structs) without requiring an explicit
/// type manager or string wrapping.
///
/// @example
///   DaqTypeFactory factory(instance);
///   daq::EnumerationPtr voltage = factory.MakeEnum("MSCL_Wireless_Voltage", "voltage_3000mV");
class DaqTypeFactory
{
public:
    /// @param instance The openDAQ instance to retrieve the type manager from.
    explicit DaqTypeFactory(daq::InstancePtr instance);

    /// @brief Creates an openDAQ Enumeration value.
    /// @param typeName The registered enum type name.
    /// @param valueName The enum value name.
    /// @return The created Enumeration.
    ///
    /// @example
    ///   daq::EnumerationPtr voltage = factory.MakeEnum("MSCL_Wireless_Voltage", "voltage_3000mV");
    daq::EnumerationPtr MakeEnum(const std::string& typeName, const std::string& valueName);

    /// @brief Creates an openDAQ Struct value.
    /// @param typeName The registered struct type name.
    /// @param fields Field name-value pairs. Values must be openDAQ types
    ///   (Integer(), Float(), Boolean(), Enumeration(), etc.).
    /// @return The created Struct.
    ///
    /// @example
    ///   daq::StructPtr eq = factory.MakeStruct("MSCL_Wireless_LinearEquation",
    ///       {{"Slope", daq::Float(1.0)}, {"Offset", daq::Float(0.0)}});
    daq::StructPtr MakeStruct(
        const std::string& typeName,
        std::initializer_list<std::pair<std::string, daq::BaseObjectPtr>> fields);

private:
    daq::TypeManagerPtr typeManager_;
};

/// @brief Inspects openDAQ registered types without requiring an explicit instance argument
/// on each call.
///
/// @example
///   DaqTypeInspector inspector(instance);
///   inspector.Describe("MSCL_Wireless_ShuntCalCmdInfo");
///   inspector.Describe("MSCL_Wireless_Voltage");
class DaqTypeInspector
{
public:
    /// @param instance The openDAQ instance to retrieve the type manager from.
    explicit DaqTypeInspector(daq::InstancePtr instance);

    /// @brief Prints the fields and values for a registered enum or struct type to stdout.
    /// @param typeName The full registered type name.
    void Describe(const std::string& typeName);

    /// @brief Prints the fields and values for a registered enum or struct type to the given stream.
    /// @param typeName The full registered type name.
    /// @param out The output stream to write to.
    void Describe(const std::string& typeName, std::ostream& out);

private:
    daq::TypeManagerPtr typeManager_;
};

/// @brief Inspects openDAQ properties from a root object using dot-notation paths.
///
/// @example
///   DaqPropertyInspector inspector(channel);
///   inspector.Describe("Setup.Configure.Sampling.LostBeaconTimeout");
class DaqPropertyInspector
{
public:
    /// @param instance The openDAQ instance to retrieve the context from.
    explicit DaqPropertyInspector(daq::InstancePtr instance);

    /// @brief Prints the type, description, and default value for a property to stdout.
    /// @param path Dot-notation path from the root to the property.
    void Describe(daq::PropertyObjectPtr root, const std::string& path);

    /// @brief Prints the type, description, and default value for a property to the given stream.
    /// @param path Dot-notation path from the root to the property.
    /// @param out The output stream to write to.
    void Describe(daq::PropertyObjectPtr root, const std::string& path, std::ostream& out);

private:
    daq::InstancePtr instance_;
};

/// @brief Returns the full dot-notation path of a property or group given its name.
/// @param root The openDAQ object to search.
/// @param name The property or group name to find.
/// @return The dot-notation path, or "Not found" if no match exists.
///
/// @example
///   std::string path = Find(channel, "LostBeaconTimeout");
///   // => "Setup.Configure.Sampling.LostBeaconTimeout"
std::string Find(daq::PropertyObjectPtr root, const std::string& name);

/// @brief Returns all property groups and subgroups as dot-notation path strings.
/// @param root The openDAQ object to query.
/// @return A list of dot-notation group paths.
std::vector<std::string> Groups(daq::PropertyObjectPtr root);

/// @brief Prints all property groups and subgroups sorted alphabetically to stdout.
/// @param root The openDAQ object to query.
void PrintGroups(daq::PropertyObjectPtr root);

/// @brief Prints all property groups and subgroups sorted alphabetically to the given stream.
/// @param root The openDAQ object to query.
/// @param out The output stream to write to.
void PrintGroups(daq::PropertyObjectPtr root, std::ostream& out);

/// @brief Returns properties as a list of PropertyInfo (group, name, type) entries.
/// @param root The openDAQ object to query.
/// @param group Optional dot-notation group path to filter properties.
/// @return A list of PropertyInfo entries.
///
/// @example
///   std::vector<PropertyInfo> props = Properties(channel, "Setup.Configure.Sampling");
std::vector<PropertyInfo> Properties(daq::PropertyObjectPtr root, const std::string& group = "");

/// @brief Prints properties as an aligned table sorted alphabetically by group then name to stdout.
/// @param root The openDAQ object to query.
/// @param group Optional dot-notation group path to filter properties.
void PrintProperties(daq::PropertyObjectPtr root, const std::string& group = "");

/// @brief Prints properties as an aligned table sorted alphabetically by group then name
/// to the given stream.
/// @param root The openDAQ object to query.
/// @param group Dot-notation group path to filter properties, or empty string for all.
/// @param out The output stream to write to.
void PrintProperties(daq::PropertyObjectPtr root, const std::string& group, std::ostream& out);

/// @brief Calls an openDAQ function property and returns its result.
/// @param root The root openDAQ property object to start from.
/// @param path The dot-notation path from the root to the function property.
/// @param args Optional arguments to pass to the function.
/// @return The result returned by the function, or an unassigned object for procedures.
///
/// @example
///   daq::BaseObjectPtr result = Call(channel, "Setup.Configure.Apply");
///   daq::BaseObjectPtr result = Call(channel, "Capabilities.ChannelType", {daq::Integer(1)});
daq::BaseObjectPtr Call(
    daq::PropertyObjectPtr root,
    const std::string& path,
    std::initializer_list<daq::BaseObjectPtr> args = {});

} // namespace daq_utils
