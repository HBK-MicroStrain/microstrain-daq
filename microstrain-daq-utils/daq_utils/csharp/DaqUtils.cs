using Daq.Core.Types;
using Daq.Core.Objects;
using Daq.Core.OpenDAQ;

namespace Daq.Utils;

/// <summary>
/// Creates openDAQ typed values (enumerations, structs) without requiring explicit type manager
/// or string wrapping.
///
/// Example:
///   var daqTypes = new DaqTypeFactory(instance);
///   var voltage = daqTypes.MakeEnum("MSCL_Wireless_Voltage", "voltage_3000mV");
/// </summary>
public class DaqTypeFactory
{
    private readonly TypeManager _typeManager;

    public DaqTypeFactory(Instance instance) =>
        _typeManager = instance.Context.TypeManager;

    /// <summary>
    /// Creates an openDAQ Enumeration value.
    ///
    /// Example:
    ///   var voltage = daqTypes.MakeEnum("MSCL_Wireless_Voltage", "voltage_3000mV");
    /// </summary>
    public Enumeration MakeEnum(string typeName, string valueName) =>
        CoreTypesFactory.CreateEnumeration(typeName, valueName, _typeManager);

    /// <summary>
    /// Creates an openDAQ Struct value. bool, int, long, float, and double fields are boxed
    /// to their openDAQ equivalents automatically; openDAQ types are passed through as-is.
    ///
    /// Example:
    ///   var eq = daqTypes.MakeStruct("MSCL_Wireless_LinearEquation",
    ///       new Dictionary&lt;string, object&gt; { ["Slope"] = 1.0, ["Offset"] = 0.0 });
    /// </summary>
    public Struct MakeStruct(string typeName, IDictionary<string, object> fields)
    {
        var daqFields = CoreTypesFactory.CreateDict<BaseObject, BaseObject>(
            fields.Select(kv => ((BaseObject)(StringObject)kv.Key, ToBaseObject(kv.Value))).ToArray()
        );
        return CoreTypesFactory.CreateStruct(typeName, daqFields, _typeManager);
    }

    private static BaseObject ToBaseObject(object value) => value switch
    {
        bool b       => (BoolObject)b,
        int i        => (IntegerObject)(long)i,
        long l       => (IntegerObject)l,
        float f      => (FloatObject)(double)f,
        double d     => (FloatObject)d,
        BaseObject o => o,
        _ => throw new ArgumentException($"Unsupported field value type: {value?.GetType().Name ?? "null"}")
    };
}

/// <summary>
/// Inspects openDAQ registered types without requiring an explicit instance argument on each call.
///
/// Example:
///   var inspector = new DaqTypeInspector(instance);
///   inspector.Describe("MSCL_Wireless_ShuntCalCmdInfo");
///   inspector.Describe("MSCL_Wireless_Voltage");
/// </summary>
public class DaqTypeInspector
{
    private readonly TypeManager _typeManager;

    public DaqTypeInspector(Instance instance) =>
        _typeManager = instance.Context.TypeManager;

    /// <summary>
    /// Prints the fields and values for a registered enum or struct type.
    ///
    /// Example:
    ///   inspector.Describe("MSCL_Wireless_AutoCalCompletionFlag");
    ///   inspector.Describe("MSCL_Wireless_LinearEquation");
    /// </summary>
    public void Describe(string typeName)
    {
        var daqType = _typeManager.GetDaqType(typeName);
        var enumType = daqType.Cast<EnumerationType>();
        if (enumType != null)
            DescribeEnum(enumType);
        else
            DescribeStruct(daqType.Cast<StructType>());
    }

    private static void DescribeEnum(EnumerationType enumType)
    {
        var names = enumType.EnumeratorNames.Select(n => (string)n).ToList();
        const string header = "Enumerator";
        var width = Math.Max(header.Length, names.Max(n => n.Length));

        Console.WriteLine();
        Console.WriteLine(header.PadRight(width));
        Console.WriteLine(new string('-', width));
        foreach (var name in names)
            Console.WriteLine(name);
        Console.WriteLine();
    }

    private static void DescribeStruct(StructType structType)
    {
        var fieldNames = structType.FieldNames.Select(n => (string)n).ToList();
        var fieldValues = structType.FieldDefaultValues.ToList();
        var rows = fieldNames.Zip(fieldValues, (name, value) => (name, DaqUtils.FieldTypeLabel(value))).ToList();

        const string col0Header = "Field";
        const string col1Header = "Type";
        var col0 = Math.Max(col0Header.Length, rows.Max(r => r.name.Length));
        var col1 = Math.Max(col1Header.Length, rows.Max(r => r.Item2.Length));

        Console.WriteLine();
        Console.WriteLine($"{col0Header.PadRight(col0)} | {col1Header.PadRight(col1)}");
        Console.WriteLine($"{new string('-', col0)}-+-{new string('-', col1)}");
        foreach (var (name, typeStr) in rows)
            Console.WriteLine($"{name.PadRight(col0)} | {typeStr.PadRight(col1)}");
        Console.WriteLine();
    }
}

public class DaqPropertyInspector
{
    private readonly Instance _instance;

    public DaqPropertyInspector(Instance instance) =>
        _instance = instance;

// Splits paths by "." and traverses known path to find the requested property. Throws if any part of the path is invalid.
    private static Property GetKnownProperty(PropertyObject root, string path)
    {
        if (string.IsNullOrWhiteSpace(path))
            throw new ArgumentException("Property path cannot be empty", nameof(path));

        string[] parts = path.Split('.');
        if (parts.Any(string.IsNullOrEmpty))
            throw new ArgumentException($"Invalid property path '{path}'", nameof(path));

        PropertyObject obj = root;
        foreach (string groupName in parts[..^1])
        {
            BaseObject groupValue = obj.GetPropertyValue(groupName);
            PropertyObject? groupObj = groupValue.Cast<PropertyObject>();
            if (groupObj == null)
                throw new InvalidOperationException($"'{groupName}' is not a property group in '{path}'");

            obj = groupObj;
        }

        return obj.GetProperty(parts[^1]);
    }

    public void Describe(PropertyObject root, string path)
    {
        Property prop = GetKnownProperty(root, path);
        string rawType = prop.ValueType.ToString();
        string typeStr = rawType.StartsWith("ct", StringComparison.Ordinal) ? rawType[2..] : rawType;

        Console.WriteLine($"Type: {typeStr}");
        Console.WriteLine($"Description: {prop.Description}");
        Console.WriteLine($"Default Value: {prop.DefaultValue} \n");
    }
}

public static class DaqUtils
{
    internal static string FieldTypeLabel(BaseObject value)
    {
        if (value == null) return "?";
        var coreTypeObj = value.Cast<CoreTypeObject>();
        if (coreTypeObj == null) return "?";
        return coreTypeObj.CoreType switch
        {
            CoreType.ctBool        => "Bool",
            CoreType.ctInt         => "Int",
            CoreType.ctFloat       => "Float",
            CoreType.ctString      => "String",
            CoreType.ctEnumeration => $"Enum<{value.Cast<Enumeration>()?.EnumerationType.Name ?? "?"}>",
            var ct                 => ct.ToString()[2..]
        };
    }

    private static IEnumerable<(PropertyObject obj, Property prop, string prefix, string path)> DepthFirstSearch(
        PropertyObject root, string prefix = "")
    {
        // Uses a stack with reversed insertion to maintain property order, keeping related subgroups
        // adjacent in output (e.g. Setup, Setup.Configure, ...) rather than breadth-first interleaving.
        var stack = new Stack<(PropertyObject obj, string prefix)>();
        stack.Push((root, prefix));

        while (stack.Count > 0)
        {
            var (obj, pre) = stack.Pop();
            var subgroups = new List<(PropertyObject subObj, string path)>();

            foreach (var prop in obj.VisibleProperties)
            {
                var path = string.IsNullOrEmpty(pre) ? prop.Name : $"{pre}.{prop.Name}";
                yield return (obj, prop, pre, path);

                if (prop.ValueType == CoreType.ctObject)
                {
                    var subObj = obj.GetPropertyValue(prop.Name).Cast<PropertyObject>();
                    if (subObj != null)
                        subgroups.Add((subObj, path));
                }
            }

            subgroups.Reverse();
            foreach (var sg in subgroups)
                stack.Push((sg.subObj, sg.path));
        }
    }

    /// <summary>
    /// Calls an openDAQ function property and returns its result.
    ///
    /// Example:
    ///   var result = DaqUtils.Call(channel, "Setup.Configure.Apply");
    ///   var result = DaqUtils.Call(channel, "Capabilities.ChannelType", (IntegerObject)1);
    ///   Console.WriteLine(result.Cast&lt;PropertyObject&gt;().GetPropertyValue("Success"));
    /// </summary>
    public static BaseObject? Call(PropertyObject root, string path, params BaseObject[] args)
    {
        var value = root.GetPropertyValue(path);
        BaseObject? param = args.Length == 0 ? null
            : args.Length == 1 ? args[0]
            : (BaseObject)CoreTypesFactory.CreateList<BaseObject>(args);

        var func = value.Cast<Function>();
        if (func != null)
            return func.Call(param);

        value.Cast<Procedure>()?.Dispatch(param);
        return null;
    }

    /// <summary>
    /// Returns the full dot-notation path of a property or group given its name.
    /// Returns "Not found" if no match exists.
    ///
    /// Example:
    ///   DaqUtils.Find(channel, "LostBeaconTimeout")
    ///   // => "Setup.Configure.Sampling.LostBeaconTimeout"
    /// </summary>
    public static string Find(PropertyObject root, string name)
    {
        foreach (var (_, prop, _, path) in DepthFirstSearch(root))
        {
            if (prop.Name == name)
                return path;
        }
        return "Not found";
    }

    /// <summary>Returns all property groups and subgroups as dot-notation path strings.</summary>
    public static List<string> Groups(PropertyObject root) =>
        DepthFirstSearch(root)
            .Where(t => t.prop.ValueType == CoreType.ctObject)
            .Select(t => t.path)
            .ToList();

    /// <summary>Prints all property groups and subgroups sorted alphabetically.</summary>
    public static void PrintGroups(PropertyObject root)
    {
        foreach (var g in Groups(root).OrderBy(g => g))
            Console.WriteLine(g);
    }

    /// <summary>
    /// Returns properties as a list of (group, name, type) tuples.
    ///
    /// Example:
    ///   var props = DaqUtils.Properties(channel, "Setup.Configure.Sampling");
    /// </summary>
    public static List<(string group, string name, string type)> Properties(PropertyObject root, string? group = null)
    {
        var startObj = group != null ? root.GetPropertyValue(group).Cast<PropertyObject>() : root;
        var startPrefix = group ?? "";

        return DepthFirstSearch(startObj, startPrefix)
            .Where(t => t.prop.ValueType != CoreType.ctObject)
            .Select(t => (
                t.prefix.Length > 0 ? t.prefix : "None",
                t.prop.Name,
                t.prop.ValueType.ToString()[2..]
            ))
            .ToList();
    }

    /// <summary>
    /// Prints properties as an aligned table sorted alphabetically by group then name.
    ///
    /// Example:
    ///   DaqUtils.PrintProperties(channel);
    ///   DaqUtils.PrintProperties(channel, "Setup.Configure.Sampling");
    /// </summary>
    public static void PrintProperties(PropertyObject root, string? group = null)
    {
        var rows = Properties(root, group).OrderBy(r => (r.group, r.name)).ToList();
        if (rows.Count == 0) return;

        const string col0Header = "Group";
        const string col1Header = "Property";
        const string col2Header = "Type";
        var col0 = Math.Max(col0Header.Length, rows.Max(r => r.group.Length));
        var col1 = Math.Max(col1Header.Length, rows.Max(r => r.name.Length));
        var col2 = Math.Max(col2Header.Length, rows.Max(r => r.type.Length));

        Console.WriteLine($"{col0Header.PadRight(col0)} | {col1Header.PadRight(col1)} | {col2Header.PadRight(col2)}");
        Console.WriteLine($"{new string('-', col0)}-+-{new string('-', col1)}-+-{new string('-', col2)}");
        foreach (var (g, n, t) in rows)
            Console.WriteLine($"{g.PadRight(col0)} | {n.PadRight(col1)} | {t.PadRight(col2)}");
    }
}
