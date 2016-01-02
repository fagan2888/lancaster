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
__copyright__ = "Copyright 2016 Leif Walsh"
__license__ = "MIT"
__version__ = "0.1.0"
__maintainer__ = "Leif Walsh"
__email__ = "leif.walsh@gmail.com"


import io

from   . import _lancaster

def read_stream(schema, stream, buffer_size=io.DEFAULT_BUFFER_SIZE):
    """Using a schema, deserialize a stream of consecutive Avro values.

    :param str schema: json string representing the Avro schema
    :param stream: a buffered stream of binary input
    :return: yields a sequence of python data structures deserialized from the stream
    """
    reader = _lancaster.Reader(schema)
    buf = stream.read(buffer_size)
    remainder = b''
    while len(buf) > 0:
        values, n = reader.read_seq(buf)
        yield from values
        remainder = buf[n:]
        try:
            buf = stream.read(buffer_size)
        except ValueError:
            break
        if len(remainder) > 0:
            if len(buf) == 0:
                break
            ba = bytearray()
            ba.extend(remainder)
            ba.extend(buf)
            buf = memoryview(ba).tobytes()
    else:
        if len(remainder) > 0:
            raise EOFError('{} bytes remaining but could not continue reading from stream'.format(len(remainder)))
