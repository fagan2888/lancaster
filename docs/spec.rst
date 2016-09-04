==============
lancaster spec
==============

Lancaster does not implement the `avro container file format`_.
Lancaster is oriented toward streaming applications where the stream
is of indeterminate length, and the avro container file format assumes
we know up front how many records we intend to deserialize.

Instead, lancaster expects to be given the schema up front, and
expects all records to conform to that schema.  Lancaster then simply
deserializes records from the stream one by one, until it reaches EOF.

The stream lancaster will deserialize from can be any binary python
`file-like object`_.  Commonly, this is a file, a UNIX socket, or an
HTTP response stream such as what `requests`_ will give you when
setting `stream=True`_.

If the stream ends in the middle of an avro record, lancaster will
raise an :class:`EOFError`.

.. _`avro container file format`: https://avro.apache.org/docs/1.8.1/spec.html#Object+Container+Files
.. _`file-like object`: https://docs.python.org/3/glossary.html#term-file-object
.. _`requests`: http://docs.python-requests.org/en/master/
.. _`stream=True`: http://docs.python-requests.org/en/master/user/advanced/#body-content-workflow

Types
-----

Almost all avro types are converted to sensible python types by
lancaster:

- `Primitive types`_ are deserialized in the obvious way, with
  ``null`` mapping to python's ``None``, integral types becoming ``int``,
  floating-point types becoming ``float``, and ``strings``, well,
  becoming ``str``.
- `Enums`_ are deserialized as python strings.
- `Unions`_ are handled, and are commonly used to represent nullable
  types, being a union of ``null`` and the field's type if present.
  ``null`` is represented as a python ``None`` value.
- `Bytes`_ and `fixed`_ types are deserialized as python
  |py_bytes|_ objects.
- `Maps`_ and `records`_ are deserialized as python
  |py_dict|_ objects, except as described below in
  `Advanced usage`_.
- `Arrays`_ are deserialized as python |py_list|_ objects.
- Recursively-defined avro schemas are not supported, and when
  lancaster encounters an avro ``link`` field, it raises a
  :class:`ValueError`.

.. _`Primitive types`: https://avro.apache.org/docs/1.8.1/spec.html#schema_primitive
.. _`Enums`: https://avro.apache.org/docs/1.8.1/spec.html#Enums
.. _`Unions`: https://avro.apache.org/docs/1.8.1/spec.html#Unions
.. _`Bytes`: https://avro.apache.org/docs/1.8.1/spec.html#schema_primitive
.. _`Fixed`: https://avro.apache.org/docs/1.8.1/spec.html#Fixed
.. |py_bytes| replace:: ``bytes``
.. _py_bytes: https://docs.python.org/3/library/functions.html#bytes
.. _`Maps`: https://avro.apache.org/docs/1.8.1/spec.html#Maps
.. _`Records`: https://avro.apache.org/docs/1.8.1/spec.html#schema_record
.. |py_dict| replace:: ``dict``
.. _py_dict: https://docs.python.org/3/library/functions.html#func-dict
.. _`Arrays`: https://avro.apache.org/docs/1.8.1/spec.html#Arrays
.. |py_list| replace:: ``list``
.. _py_list: https://docs.python.org/3/library/functions.html#func-list

Advanced usage
--------------

Lancaster truly shines when deserializing non-nested `avro records`_.
A stream of non-nested records nicely represents tabular data.  In
this case, lancaster supports a secondary method of deserializing
data, producing tuples instead of dicts, which is more efficient with
CPU and memory.  In this case, it is important to keep track of the
field ordering in the avro schema, because that ordering is the only
way to map fields in the deserialized tuple back to the names in the
avro schema.

In addition, when deserializing tuples this way, fields of avro type
``long`` can be converted to python |py_datetime|_ objects, and are
assumed to be nanoseconds since the UNIX epoch.  Lancaster handles
this whenever the avro schema contains ``{'is_datetime': true}``
inside the type specification for a field of type ``long``.  Nullable
fields of type ``long`` (which are, in avro, of type ``["null",
"long"]``), are also acceptable, in this case any ``null`` values will
still be deserialized as ``None``.

Both of these features are available when using the
:func:`lancaster.read_stream_tuples` API instead of
:func:`lancaster.read_stream`.

.. _`avro records`: https://avro.apache.org/docs/1.8.1/spec.html#schema_record
.. |py_datetime| replace:: ``datetime``
.. _py_datetime: https://docs.python.org/3/library/datetime.html
