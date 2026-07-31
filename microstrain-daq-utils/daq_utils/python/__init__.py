import opendaq as daq


class DaqTypeFactory:
    """Creates openDAQ typed values (enumerations, structs) without requiring explicit type manager or string wrapping.

    Args:
        instance: The openDAQ Instance to retrieve the type manager from.

    Example:
        daq_types = daq_utils.DaqTypeFactory(instance)
        voltage = daq_types.enum("MSCL_Wireless_Voltage", "voltage_3000mV")
    """

    def __init__(self, instance):
        self._instance = instance
        self._type_manager = instance.context.type_manager

    def enum(self, type_name, value_name):
        """Creates an openDAQ Enumeration value.

        Args:
            type_name: The openDAQ enum type name.
            value_name: The enum value name.

        Example:
            voltage = daq_types.enum("MSCL_Wireless_Voltage", "voltage_3000mV")
        """
        return daq.Enumeration(daq.String(type_name), daq.String(value_name), self._type_manager)

    def struct(self, type_name, fields):
        """Creates an openDAQ Struct value.

        Args:
            type_name: The openDAQ struct type name.
            fields: A dict containing the struct's field names and values. Python primitives
                (bool, int, float) are converted to their openDAQ equivalents automatically.
                openDAQ types (e.g. Enumeration) are passed through as-is.

        Example:
            linear_eq = daq_types.struct("MSCL_Wireless_LinearEquation", {"Slope": 1.0, "Offset": 0.0})
        """
        daq_fields = daq.Dict()

        for key, value in fields.items():
            if isinstance(value, bool):
                daq_fields[key] = daq.Boolean(value)
            elif isinstance(value, int):
                daq_fields[key] = daq.Integer(value)
            elif isinstance(value, float):
                daq_fields[key] = daq.Float(value)
            else:
                daq_fields[key] = value

        return daq.Struct(daq.String(type_name), daq_fields, self._type_manager)


class DaqTypeInspector:
    """Inspects openDAQ registered types without requiring an explicit instance argument on each call.

    Args:
        instance: The openDAQ Instance to retrieve the type manager from.

    Example:
        inspector = daq_utils.DaqTypeInspector(instance)
        inspector.describe("MSCL_Wireless_ShuntCalCmdInfo")
        inspector.describe("MSCL_Wireless_Voltage")
    """

    def __init__(self, instance):
        self._instance = instance
        # Releasing an IEnumerationType Python wrapper corrupts the underlying C++ object in the
        # openDAQ Python binding, causing a segfault on any subsequent call for the same type. Caching
        # the names as plain Python strings after the first call avoids touching the C++ object again.
        #
        # NOTE: Temporary workaround — remove once the openDAQ version is updated with a fix.
        self._enum_name_cache: dict[str, list[str]] = {}

    def describe(self, type_name):
        """Prints the fields and values for a registered enum or struct type.

        Args:
            type_name: The full registered type name.

        Example:
            inspector.describe("MSCL_Wireless_AutoCalCompletionFlag")
            inspector.describe("MSCL_Wireless_LinearEquation")
        """
        type_obj = self._instance.context.type_manager.get_type(type_name)

        if daq.IEnumerationType.can_cast_from(type_obj):
            self._describe_enum(type_name, type_obj)
        else:
            self._describe_struct(type_name, type_obj)

    def _describe_enum(self, type_name, type_obj):
        if type_name not in self._enum_name_cache:
            enum_type = daq.IEnumerationType.cast_from(type_obj)
            self._enum_name_cache[type_name] = [str(k) for k in enum_type.as_dictionary]

        names = self._enum_name_cache[type_name]
        header = 'Enumerator'
        width = max(len(header), max(len(str(n)) for n in names))

        print()
        print(f'{header:<{width}}')
        print('-' * width)

        for name in names:
            print(str(name))

        print()

    def _describe_struct(self, type_name, type_obj):
        struct_type = daq.IStructType.cast_from(type_obj)
        builder = daq.StructBuilder(daq.String(type_name), self._instance.context.type_manager)

        rows = [
            (str(name), _field_type_label(value))
            for name, value in zip(struct_type.field_names, builder.field_values)
        ]
        headers = ('Field', 'Type')
        col_widths = [max(len(r[i]) for r in rows + [headers]) for i in range(2)]

        print()
        print(f'{headers[0]:<{col_widths[0]}} | {headers[1]:<{col_widths[1]}}')
        print(f'{"-" * col_widths[0]}-+-{"-" * col_widths[1]}')

        for name, type_str in rows:
            print(f'{name:<{col_widths[0]}} | {type_str:<{col_widths[1]}}')

        print()


class DaqPropertyInspector:
    def __init__(self, instance):
        self._instance = instance

    # Splits paths by "." and traverses known path to find the requested property. Throws if any part of the path is invalid.
    def _get_known_property(self, root, path):
        parts = path.split(".")
        obj = root

        for group_name in parts[:-1]:
            obj = obj.get_property_value(group_name)
            if not daq.IPropertyObject.can_cast_from(obj):
                raise RuntimeError(f"'{group_name}' is not a property group in '{path}'")
            obj = daq.IPropertyObject.cast_from(obj)

        return obj.get_property(parts[-1])

    def describe(self, root, path):
        prop = self._get_known_property(root, path)

        raw = str(prop.value_type)
        type_str = raw.split("CoreType.")[-1]
        if type_str.startswith("ct"):
            type_str = type_str[2:]

        rows = [
            ("Type", type_str),
            ("Description", str(prop.description)),
            ("Default value", str(prop.default_value)),
        ]
        col0 = max(len(label) for label, _ in rows)
        col1 = max(len(value) for _, value in rows)

        print()
        for label, value in rows:
            print(f'{label:<{col0}} | {value:<{col1}}')
        print(f'{"-" * col0}-+-{"-" * col1}')
        print()


def _field_type_label(value):
    if value is None:
        return '?'
    if isinstance(value, bool):
        return 'Bool'
    if isinstance(value, int):
        return 'Int'
    if isinstance(value, float):
        return 'Float'
    if isinstance(value, str):
        return 'String'
    if daq.IEnumeration.can_cast_from(value):
        return f'Enumeration[{daq.IEnumeration.cast_from(value).enumeration_type.name}]'
    try:
        return f'Struct[{value.struct_type.name}]'
    except AttributeError:
        return type(value).__name__


def _prop_type_label(obj, prop):
    vt = prop.value_type
    if vt == daq.CoreType.ctBool:
        return 'Bool'
    if vt == daq.CoreType.ctInt:
        return 'Int'
    if vt == daq.CoreType.ctFloat:
        return 'Float'
    if vt == daq.CoreType.ctString:
        return 'String'
    if vt == daq.CoreType.ctEnumeration:
        try:
            value = obj.get_property_value(prop.name)
            return f'Enumeration[{daq.IEnumeration.cast_from(value).enumeration_type.name}]'
        except Exception:
            return 'Enumeration'
    if vt == daq.CoreType.ctStruct:
        try:
            return f'Struct[{obj.get_property_value(prop.name).struct_type.name}]'
        except Exception:
            return 'Struct'
    if vt == daq.CoreType.ctList:
        try:
            items = obj.get_property_value(prop.name)
            first = next(iter(items), None)
            if first is not None:
                return f'List<{_field_type_label(first)}>'
        except Exception:
            pass
        return 'List'
    if vt == daq.CoreType.ctDict:
        return 'Dict'
    raw = str(vt).split('CoreType.')[-1]
    return raw[2:] if raw.startswith('ct') else raw


def call(root, path, *args):
    """Calls an openDAQ function property and return its result.

    This function handles casting to a callable automatically.

    Args:
        root: The root openDAQ property object to start from.
        path: The name or dot-notation path from the root to the function property.
        *args: Optional arguments to pass to the function property.

    Returns:
        The result returned by the function property, typically a PropertyObject.

    Raises:
        RuntimeError: If the property does not exist or cannot be cast properly.

    Example:
        # Function with no arguments
        result = call(channel, "Setup.Configure.Apply")

        # Function with arguments
        result = call(channel, "Capabilities.ChannelType", 1)

        # Query the result
        print(result.get_property_value("Success"))
    """
    return daq.IFunction.cast_from(root.get_property_value(path))(*args)


def _depth_first_search(root, prefix=''):
    """Depth-first search generator that yields (obj, prop, prefix, path) for every visible property."""

    # Uses a stack with reversed insertion to maintain original property order. This keeps related subgroups
    # adjacent in output (e.g. [Setup, Setup.Configure, Setup.Configure.Sampling, ...]) vs. breadth-first which
    # interleaves levels (e.g. [Setup, Capabilities, ..., Setup.Configure, ...]).
    stack = [(root, prefix)]

    while stack:
        obj, prefix = stack.pop()
        subgroups = []

        for prop in obj.visible_properties:
            path = f'{prefix}.{prop.name}' if prefix else prop.name
            yield obj, prop, prefix, path
            if prop.value_type == daq.CoreType.ctObject:
                subgroups.append((obj.get_property_value(prop.name), path))

        subgroups.reverse()
        stack.extend(subgroups)


def find(root, name):
    """Returns the full dot-notation path of a property or group given its name.

    Searches all groups and subgroups. Returns 'Not found' if no match is found.

    Args:
        root: The openDAQ object to search (channel, device, group, etc.).
        name: The property or group name to search for.

    Example:
        find(channel, 'LostBeaconTimeout')
        # => 'Setup.Configure.Sampling.LostBeaconTimeout'

        find(channel, 'Sampling')
        # => 'Setup.Configure.Sampling'
    """
    return next(
        (path for _, prop, _, path in _depth_first_search(root) if prop.name == name),
        'Not found'
    )


def groups(root):
    """Returns all property groups and subgroups as dot-notation path strings.

    Args:
        root: The openDAQ object to query property groups from (channel, device, group, etc.).
    """
    return [
        path for _, prop, _, path in _depth_first_search(root)
        if prop.value_type == daq.CoreType.ctObject
    ]


def print_groups(root):
    """Prints all property groups and subgroups using dot-notation paths, sorted alphabetically.

    Args:
        root: The openDAQ object to query property groups from (channel, device, group, etc.).
    """
    print('\n'.join(sorted(groups(root))))


def properties(root, group=None):
    """Returns properties as a list of (group, name, type) tuples.

    Args:
        root: The openDAQ object to query properties from.
        group: Optional dot-notation group path to filter properties.
    """
    start_obj = root.get_property_value(group) if group else root
    start_prefix = group or ''

    return [
        (prefix or 'None', prop.name, _prop_type_label(obj, prop))
        for obj, prop, prefix, _ in _depth_first_search(start_obj, start_prefix)
        if prop.value_type != daq.CoreType.ctObject
    ]


def print_properties(root, group=None):
    """Prints properties as an aligned table, sorted alphabetically by group then property name.

    Args:
        root: The openDAQ object to query properties from.
        group: Optional dot-notation group path to filter properties.
    """
    rows = sorted(properties(root, group), key=lambda r: (r[0], r[1]))
    headers = ('Group', 'Property', 'Type')
    col_widths = [max(len(r[i]) for r in rows + [headers]) for i in range(3)]

    print(f'{headers[0]:<{col_widths[0]}} | {headers[1]:<{col_widths[1]}} | {headers[2]:<{col_widths[2]}}')
    print(f'{"-" * col_widths[0]}-+-{"-" * col_widths[1]}-+-{"-" * col_widths[2]}')

    for row in rows:
        print(f'{row[0]:<{col_widths[0]}} | {row[1]:<{col_widths[1]}} | {row[2]:<{col_widths[2]}}')
