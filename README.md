lancaster
=========

A python extension wrapper for [avro-c][avro-c].

Currently only supports reading a stream of avro serialized data.
Does not support writing, nor the avro container format.

See also [the Avro project page][avro].

Usage
-----

    schema = '{ ... }'
    with open('data.avro', 'rb') as f:
        data = list(lancaster.read_stream(schema, f))

`lancaster.read_stream()` accepts a json string describing the schema,
and a stream to read from, and returns a generator which will produce
python versions of the avro data (dicts, lists, ints, strings).

Installation
------------

A conda package is provided at [anaconda.org][anaconda].  This depends
on conda packages providing the C libraries required, `libsnappy`,
`jansson`, and `libavro-c`.

    conda create -n lancaster -c leif python lancaster

You can also install using just setuptools, assuming you have `avro-c`
installed on your system, which you can probably get from your OS
package manager.

    git clone https://github.com/twosigma/lancaster
    cd lancaster
    python setup.py install

Caveats
-------

Recursive structures (links) are not supported.  Writing anything, and
the avro container file format, are also not supported.  Happy to
accept pull requests but I don't need those features personally yet.

License
-------

Copyright 2016 Two Sigma Open Source, LLC.  [MIT licensed][license].

[anaconda]: https://anaconda.org/leif/lancaster
[avro]: https://avro.apache.org/
[avro-c]: http://avro.apache.org/docs/1.7.7/api/c/index.html
[license]: https://github.com/twosigma/lancaster/blob/master/LICENSE
