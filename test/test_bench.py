#!/usr/bin/env python

from   datetime import datetime
import io
import json
import os
import tempfile
import random
import string
import time

import avro.io
import avro.schema

import lancaster

# The avro-python3 library sorts these for you, so you can get
# different data than you should.  The avro-c library correctly leaves
# the order alone.  We sort beforehand so that we're guaranteed to get
# the same answer as what the python lib writes out so our test
# passes.  AVRO-1673
enum_vals = sorted(['CLUBS', 'HEARTS', 'SPADES', 'DIAMONDS'])

schema = '''{"type":"record",
  "name": "Person",
  "fields": [
     {"name": "ID", "type": "long"},
     {"name": "First", "type": "string"},
     {"name": "Last", "type": "string"},
     {"name": "Birthday", "type": "long", "is_datetime": true},
     {"name": "Phone", "type": "string"},
     {"name": "Age", "type": "int"},
     {"name": "Suit", "type": {"type": "enum", "name": "suits", "symbols": [%s]}}]}''' % ','.join('"{}"'.format(v) for v in enum_vals)

datetime_flags = {field['name']: field.get('is_datetime', False) for field in json.loads(schema)['fields']}

def convert_nanos_to_datetime(val):
    t = datetime.fromtimestamp(val * 1e-9)
    return t.replace(microsecond = int(val % 1000000000 / 1000))

def gen_person(i):
    return {
        'ID':       i,
        'First':    ''.join(random.sample(string.ascii_letters, 12)),
        'Last':     ''.join(random.sample(string.ascii_letters, 15)),
        'Birthday': random.randint(1000000000000000000, 2000000000000000000),
        'Phone':    ''.join(random.sample(string.digits, 10)),
        'Age':      random.choice(range(100)),
        'Suit':     random.choice(enum_vals)
    }

def gen_data(n, f):
    writer = avro.io.DatumWriter(avro.schema.Parse(schema))
    encoder = avro.io.BinaryEncoder(f)
    for i in range(n):
        writer.write(gen_person(i), encoder)

def avro_read(n, f):
    reader = avro.io.DatumReader(avro.schema.Parse(schema))
    decoder = avro.io.BinaryDecoder(f)
    for i in range(n):
        yield reader.read(decoder)

def lancaster_read(f):
    yield from lancaster.read_stream(schema, f)

def lancaster_read_tuples(f):
    yield from lancaster.read_stream_tuples(schema, f)

class Timer(object):
    def __enter__(self):
        self.start = time.clock()
        return self
    def __exit__(self, *args):
        self.end = time.clock()
    @property
    def interval(self):
        return self.end - self.start

def test_main(N=10000):
    fd, filename = tempfile.mkstemp()
    try:
        with Timer() as t:
            with os.fdopen(fd, 'wb') as f:
                gen_data(N, f)
        filesize = os.stat(filename).st_size

        print('avro-py3  wrote {} values'.format(N))
        print('  in {:3.02f}s ({:3.02f}us per value / {:3.02f}MB/s)'.format(
            t.interval, 1000000 * t.interval/N, filesize / t.interval / 1024 / 1024))

        with Timer() as at:
            with open(filename, 'rb') as f:
                avro_values = list(avro_read(N, f))

        print('avro-py3  read  {} values like {}'.format(len(avro_values), avro_values[0]))
        print('  in {:3.02f}s ({:3.02f}us per value / {:3.02f}MB/s)'.format(
            at.interval, 1000000 * at.interval/len(avro_values), filesize / at.interval / 1024 / 1024))

        with Timer() as lt:
            with open(filename, 'rb') as f:
                lancaster_values = list(lancaster_read(f))

        print('lancaster read  {} values like {}'.format(len(lancaster_values), lancaster_values[0]))
        print('  in {:3.02f}s ({:3.02f}us per value / {:3.02f}MB/s)'.format(
            lt.interval, 1000000 * lt.interval/len(lancaster_values), filesize / lt.interval / 1024 / 1024))

        with Timer() as lt2:
            with open(filename, 'rb') as f:
                lancaster_tuples = list(lancaster_read_tuples(f))

        print('lancaster read  {} tuples like {}'.format(len(lancaster_tuples), lancaster_tuples[0]))
        print('  in {:3.02f}s ({:3.02f}us per value / {:3.02f}MB/s)'.format(
            lt2.interval, 1000000 * lt2.interval/len(lancaster_tuples), filesize / lt2.interval / 1024 / 1024))

        print('avro-py3 takes {:3.02f}x as long as lancaster'.format(at.interval / lt.interval))
        print('avro-py3 takes {:3.02f}x as long as lancaster to tuples'.format(at.interval / lt2.interval))

        assert len(avro_values) == len(lancaster_values)
        assert len(avro_values) == len(lancaster_tuples)
        for av, lv in zip(avro_values, lancaster_values):
            for k, v in av.items():
                assert k in lv
                assert lv[k] == (convert_nanos_to_datetime(v) if datetime_flags[k] else v)
            for k, v in lv.items():
                assert k in av
                assert (convert_nanos_to_datetime(av[k]) if datetime_flags[k] else av[k]) == v
    finally:
        os.remove(filename)

if __name__ == '__main__':
    import sys
    if len(sys.argv) > 1:
        test_main(int(sys.argv[1]))
    else:
        test_main()
