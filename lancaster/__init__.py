"""Fast deserialization of Avro binary data from a stream.

The pure python Avro library is very slow.  There is a project,
fastavro, which implements a C extension to speed this up, but it is
restricted to the Avro container format.  This library ignores the
container format and assumes input is a stream of consecutive
serialized values, of unknown size.

lancaster does not support writing, nor recursive data structures.

:Example:

with open('data.avro', 'rb') as f:
    schema = '{ ... }'
    data = list(lancaster.read_stream(schema, f))

"""

__author__ = "Leif Walsh"
__copyright__ = "Copyright 2016 Two Sigma Open Source, LLC."
__license__ = "MIT"
__version__ = "0.3.0"
__maintainer__ = "Leif Walsh"
__email__ = "leif@twosigma.com"


import io

from   . import _lancaster


def read_stream(schema, stream, *, buffer_size=io.DEFAULT_BUFFER_SIZE, datetime_flags=None):
    """Using a schema, deserialize a stream of consecutive Avro values.

    :param str schema: json string representing the Avro schema
    :param stream: a buffered stream of binary input
    :param buffer_size: size of bytes to read from the stream each time
    :param datetime_flags: a list of boolean flags determining if whether a field is a long value of nanoseconds representing a datetime value or not
    :return: yields a sequence of python data structures deserialized from the stream
    """
    if datetime_flags is not None:
        reader = _lancaster.Reader(schema, datetime_flags)
    else:
        reader = _lancaster.Reader(schema)
    buf = stream.read(buffer_size)
    remainder = b''
    while len(buf) > 0:
        values, n = reader.read_seq(buf)
        yield from values
        remainder = buf[n:]
        buf = stream.read(buffer_size)
        if len(buf) > 0 and len(remainder) > 0:
            ba = bytearray()
            ba.extend(remainder)
            ba.extend(buf)
            buf = memoryview(ba).tobytes()
    if len(remainder) > 0:
        raise EOFError('{} bytes remaining but could not continue reading from stream'.format(len(remainder)))


def read_stream_tuples(schema, stream, *, buffer_size=io.DEFAULT_BUFFER_SIZE, datetime_flags=None):
    """Using a schema, deserialize a stream of consecutive Avro values
    into tuples.

    This assumes the input is avro records of simple values (numbers,
    strings, etc.).

    :param str schema: json string representing the Avro schema
    :param stream: a buffered stream of binary input
    :param buffer_size: size of bytes to read from the stream each time
    :param datetime_flags: a list of boolean flags determining if whether a field is a long value of nanoseconds representing a datetime value or not
    :return: yields a sequence of python tuples deserialized from the stream

    """
    if datetime_flags is not None:
        reader = _lancaster.Reader(schema, datetime_flags)
    else:
        reader = _lancaster.Reader(schema)
    buf = stream.read(buffer_size)
    remainder = b''
    while len(buf) > 0:
        values, n = reader.read_seq_tuples(buf)
        yield from values
        remainder = buf[n:]
        buf = stream.read(buffer_size)
        if len(buf) > 0 and len(remainder) > 0:
            ba = bytearray()
            ba.extend(remainder)
            ba.extend(buf)
            buf = memoryview(ba).tobytes()
    if len(remainder) > 0:
        raise EOFError('{} bytes remaining but could not continue reading from stream'.format(len(remainder)))
