# lancaster spec

Lancaster does not implement the [avro container file format][1].
Lancaster is oriented toward streaming applications where the stream
is of indeterminate length, and the avro container file format assumes
we know up front how many records we intend to deserialize.

Instead, lancaster expects to be given the schema up front, and
expects all records to conform to that schema.  Lancaster then simply
deserializes records from the stream one by one, until it reaches EOF.

The stream lancaster will deserialize from can be any binary python
[file-like object][2].  Commonly, this is a file, a UNIX socket, or an
HTTP response stream such as what [requests][3] will give you when
setting [`stream=True`][4].

If the stream ends in the middle of an avro record, lancaster will
raise an `EOFError`.

[1]: https://avro.apache.org/docs/1.8.1/spec.html#Object+Container+Files
[2]: https://docs.python.org/3/glossary.html#term-file-object
[3]: http://docs.python-requests.org/en/master/
[4]: http://docs.python-requests.org/en/master/user/advanced/#body-content-workflow

## Types

Almost all avro types are converted to sensible python types by
lancaster:

- [Primitive types][5] are deserialized in the obvious way, with
  `null` mapping to python's `None`, integral types becoming `int`s,
  floating-point types becoming `float`s, and `strings`, well,
  becoming `string`s.
- [Enums][6] are deserialized as python strings.
- [Unions][7] are handled, and are commonly used to represent nullable
  types, being a union of `null` and the field's type if present.
  `null` is represented as a python `None` value.
- [Bytes][8] and [fixed][9] types are deserialized as python
  [`bytes`][10] objects.
- [Maps][11] and [records][12] are deserialized as python
  [`dict`][13]s, except as described below in
  [Advanced usage](#advanced-usage).
- [Arrays][14] are deserialized as python [`list`][15] objects.
- Recursively-defined avro schemas are not supported, and when
  lancaster encounters an avro `link` field, it raises a `ValueError`.

[5]: https://avro.apache.org/docs/1.8.1/spec.html#schema_primitive
[6]: https://avro.apache.org/docs/1.8.1/spec.html#Enums
[7]: https://avro.apache.org/docs/1.8.1/spec.html#Unions
[8]: https://avro.apache.org/docs/1.8.1/spec.html#schema_primitive
[9]: https://avro.apache.org/docs/1.8.1/spec.html#Fixed
[10]: https://docs.python.org/3/library/functions.html#bytes
[11]: https://avro.apache.org/docs/1.8.1/spec.html#Maps
[12]: https://avro.apache.org/docs/1.8.1/spec.html#schema_record
[13]: https://docs.python.org/3/library/functions.html#func-dict
[14]: https://avro.apache.org/docs/1.8.1/spec.html#Arrays
[15]: https://docs.python.org/3/library/functions.html#func-list

## Advanced usage

Lancaster truly shines when deserializing non-nested
[avro records][16].  A stream of non-nested records nicely represents
tabular data.  In this case, lancaster supports a secondary method of
deserializing data, producing tuples instead of dicts, which is more
efficient with CPU and memory.  In this case, it is important to keep
track of the field ordering in the avro schema, because that ordering
is the only way to map fields in the deserialized tuple back to the
names in the avro schema.

In addition, when deserializing tuples this way, fields of avro type
`long` can be converted to python [`datetime`][17] objects, and are
assumed to be nanoseconds since the UNIX epoch.  Lancaster handles
this whenever the avro schema contains `{'is_datetime': true}` inside
the type specification for a field of type `long`.  Nullable fields of
type `long` (which are, in avro, of type `["null", "long"]`), are also
acceptable, in this case any `null` values will still be deserialized
as `None`.

Both of these features are available when using the
`lancaster.read_stream_tuples()` API instead of
`lancaster.read_stream()`.

[16]: https://avro.apache.org/docs/1.8.1/spec.html#schema_record
[17]: https://docs.python.org/3/library/datetime.html
