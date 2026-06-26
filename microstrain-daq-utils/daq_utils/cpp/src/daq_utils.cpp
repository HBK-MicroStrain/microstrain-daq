#include <algorithm>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include <opendaq/opendaq.h>

#include <daq_utils/daq_utils.h>

namespace daq_utils {

namespace detail {

std::string CoreTypeToString(daq::CoreType type)
{
    switch (type)
    {
        case daq::CoreType::ctBool:          return "Bool";
        case daq::CoreType::ctInt:           return "Int";
        case daq::CoreType::ctFloat:         return "Float";
        case daq::CoreType::ctString:        return "String";
        case daq::CoreType::ctList:          return "List";
        case daq::CoreType::ctDict:          return "Dict";
        case daq::CoreType::ctRatio:         return "Ratio";
        case daq::CoreType::ctProc:          return "Proc";
        case daq::CoreType::ctObject:        return "Object";
        case daq::CoreType::ctBinaryData:    return "BinaryData";
        case daq::CoreType::ctFunc:          return "Func";
        case daq::CoreType::ctComplexNumber: return "ComplexNumber";
        case daq::CoreType::ctStruct:        return "Struct";
        case daq::CoreType::ctEnumeration:   return "Enumeration";
        case daq::CoreType::ctUndefined:     return "Undefined";
        default:                             return "Unknown";
    }
}

std::string FieldTypeLabel(const daq::BaseObjectPtr& value)
{
    if (!value.assigned())
    {
        return "?";
    }

    daq::CoreType ct = value.getCoreType();
    if (ct == daq::CoreType::ctEnumeration)
    {
        daq::EnumerationPtr e = value.asPtr<daq::IEnumeration>();
        return "Enum<" + std::string(e.getEnumerationType().getName()) + ">";
    }
    return CoreTypeToString(ct);
}

// Visits every visible property in depth-first order, keeping related subgroups adjacent
// in output (e.g. Setup, Setup.Configure, ...) rather than breadth-first interleaving.
// Visitor is called as visit(obj, prop, prefix, path).
template <typename Visitor>
void DepthFirstSearch(daq::PropertyObjectPtr root, std::string prefix, Visitor&& visit)
{
    struct Frame
    {
        daq::PropertyObjectPtr obj;
        std::string prefix;
    };

    std::vector<Frame> stack;
    stack.push_back({root, std::move(prefix)});

    while (!stack.empty())
    {
        Frame frame = std::move(stack.back());
        stack.pop_back();

        std::vector<Frame> subgroups;

        for (daq::PropertyPtr prop : frame.obj.getVisibleProperties())
        {
            std::string propName = prop.getName();
            std::string path = frame.prefix.empty() ? propName : frame.prefix + "." + propName;

            visit(frame.obj, prop, frame.prefix, path);

            if (prop.getValueType() == daq::CoreType::ctObject)
            {
                daq::PropertyObjectPtr sub =
                    frame.obj.getPropertyValue(daq::String(propName)).asPtr<daq::IPropertyObject>();
                if (sub.assigned())
                {
                    subgroups.push_back({std::move(sub), std::move(path)});
                }
            }
        }

        for (auto it = subgroups.rbegin(); it != subgroups.rend(); ++it)
        {
            stack.push_back(std::move(*it));
        }
    }
}

} // namespace detail


DaqTypeFactory::DaqTypeFactory(daq::InstancePtr instance)
    : typeManager_(instance.getContext().getTypeManager())
{
}

daq::EnumerationPtr DaqTypeFactory::MakeEnum(const std::string& typeName, const std::string& valueName)
{
    return daq::Enumeration(daq::String(typeName), daq::String(valueName), typeManager_);
}

daq::StructPtr DaqTypeFactory::MakeStruct(
    const std::string& typeName,
    std::initializer_list<std::pair<std::string, daq::BaseObjectPtr>> fields)
{
    daq::DictPtr<daq::IString, daq::IBaseObject> dict = daq::Dict<daq::IString, daq::IBaseObject>();
    for (const std::pair<std::string, daq::BaseObjectPtr>& field : fields)
    {
        dict.set(daq::String(field.first), field.second);
    }
    return daq::Struct(daq::String(typeName), dict, typeManager_);
}


DaqTypeInspector::DaqTypeInspector(daq::InstancePtr instance)
    : typeManager_(instance.getContext().getTypeManager())
{
}

void DaqTypeInspector::Describe(const std::string& typeName)
{
    Describe(typeName, std::cout);
}

void DaqTypeInspector::Describe(const std::string& typeName, std::ostream& out)
{
    daq::TypePtr daqType = typeManager_.getType(daq::String(typeName));
    daq::EnumerationTypePtr enumType = daqType.asPtrOrNull<daq::IEnumerationType>();
    if (enumType.assigned())
    {
        daq::ListPtr<daq::IString> names = enumType.getEnumeratorNames();
        const std::string header = "Enumerator";
        size_t width = header.size();
        for (daq::StringPtr name : names)
        {
            width = std::max(width, std::string(name).size());
        }

        out << "\n" << header << std::string(width - header.size(), ' ') << "\n";
        out << std::string(width, '-') << "\n";
        for (daq::StringPtr name : names)
        {
            out << std::string(name) << "\n";
        }
        out << "\n";
    }
    else
    {
        daq::StructTypePtr structType = daqType.asPtr<daq::IStructType>();
        daq::ListPtr<daq::IString> fieldNames = structType.getFieldNames();
        daq::ListPtr<daq::IBaseObject> fieldDefaults = structType.getFieldDefaultValues();

        const std::string col0Header = "Field";
        const std::string col1Header = "Type";
        size_t col0 = col0Header.size();
        size_t col1 = col1Header.size();

        std::vector<std::pair<std::string, std::string>> rows;
        for (size_t i = 0; i < fieldNames.getCount(); ++i)
        {
            std::string fieldName = fieldNames[i];
            std::string typeStr = detail::FieldTypeLabel(fieldDefaults[i]);
            col0 = std::max(col0, fieldName.size());
            col1 = std::max(col1, typeStr.size());
            rows.push_back({std::move(fieldName), std::move(typeStr)});
        }

        auto pad = [](const std::string& s, size_t w) { return s + std::string(w - s.size(), ' '); };

        out << "\n";
        out << pad(col0Header, col0) << " | " << pad(col1Header, col1) << "\n";
        out << std::string(col0, '-') << "-+-" << std::string(col1, '-') << "\n";
        for (const std::pair<std::string, std::string>& row : rows)
        {
            out << pad(row.first, col0) << " | " << row.second << "\n";
        }
        out << "\n";
    }
}


std::string Find(daq::PropertyObjectPtr root, const std::string& name)
{
    std::string result = "Not found";
    detail::DepthFirstSearch(
        root,
        "",
        [&](const daq::PropertyObjectPtr&, daq::PropertyPtr prop,
            const std::string&, const std::string& path)
        {
            if (result == "Not found" && std::string(prop.getName()) == name)
            {
                result = path;
            }
        });
    return result;
}

std::vector<std::string> Groups(daq::PropertyObjectPtr root)
{
    std::vector<std::string> result;
    detail::DepthFirstSearch(
        root,
        "",
        [&](const daq::PropertyObjectPtr&, daq::PropertyPtr prop,
            const std::string&, const std::string& path)
        {
            if (prop.getValueType() == daq::CoreType::ctObject)
            {
                result.push_back(path);
            }
        });
    return result;
}

void PrintGroups(daq::PropertyObjectPtr root)
{
    PrintGroups(root, std::cout);
}

void PrintGroups(daq::PropertyObjectPtr root, std::ostream& out)
{
    std::vector<std::string> g = Groups(root);
    std::sort(g.begin(), g.end());
    for (const std::string& group : g)
    {
        out << group << "\n";
    }
}

std::vector<PropertyInfo> Properties(daq::PropertyObjectPtr root, const std::string& group)
{
    daq::PropertyObjectPtr startObj = group.empty()
        ? root
        : root.getPropertyValue(daq::String(group)).asPtr<daq::IPropertyObject>();

    std::vector<PropertyInfo> result;
    detail::DepthFirstSearch(
        startObj,
        group,
        [&](const daq::PropertyObjectPtr&, daq::PropertyPtr prop,
            const std::string& prefix, const std::string&)
        {
            if (prop.getValueType() != daq::CoreType::ctObject)
            {
                result.push_back({
                    prefix.empty() ? "None" : prefix,
                    std::string(prop.getName()),
                    detail::CoreTypeToString(prop.getValueType())
                });
            }
        });
    return result;
}

void PrintProperties(daq::PropertyObjectPtr root, const std::string& group)
{
    PrintProperties(root, group, std::cout);
}

void PrintProperties(daq::PropertyObjectPtr root, const std::string& group, std::ostream& out)
{
    std::vector<PropertyInfo> rows = Properties(root, group);
    if (rows.empty())
    {
        return;
    }

    std::sort(rows.begin(), rows.end(), [](const PropertyInfo& a, const PropertyInfo& b)
    {
        return a.group != b.group ? a.group < b.group : a.name < b.name;
    });

    size_t col0 = 5;  // "Group"
    size_t col1 = 8;  // "Property"
    size_t col2 = 4;  // "Type"
    for (const PropertyInfo& r : rows)
    {
        col0 = std::max(col0, r.group.size());
        col1 = std::max(col1, r.name.size());
        col2 = std::max(col2, r.type.size());
    }

    auto pad = [](const std::string& s, size_t w) { return s + std::string(w - s.size(), ' '); };

    out << pad("Group", col0) << " | " << pad("Property", col1) << " | " << pad("Type", col2) << "\n";
    out << std::string(col0, '-') << "-+-" << std::string(col1, '-') << "-+-" << std::string(col2, '-') << "\n";
    for (const PropertyInfo& r : rows)
    {
        out << pad(r.group, col0) << " | " << pad(r.name, col1) << " | " << pad(r.type, col2) << "\n";
    }
}

daq::BaseObjectPtr Call(
    daq::PropertyObjectPtr root,
    const std::string& path,
    std::initializer_list<daq::BaseObjectPtr> args)
{
    daq::BaseObjectPtr value = root.getPropertyValue(daq::String(path));

    daq::BaseObjectPtr param;
    if (args.size() == 1)
    {
        param = *args.begin();
    }
    else if (args.size() > 1)
    {
        daq::ListPtr<daq::IBaseObject> list = daq::List<daq::IBaseObject>();
        for (const daq::BaseObjectPtr& arg : args)
        {
            list.pushBack(arg);
        }
        param = list;
    }

    daq::FunctionPtr func = value.asPtrOrNull<daq::IFunction>();
    if (func.assigned())
    {
        return func(param);
    }

    daq::ProcedurePtr proc = value.asPtrOrNull<daq::IProcedure>();
    if (proc.assigned())
    {
        proc(param);
    }

    return daq::BaseObjectPtr{};
}

} // namespace daq_utils
