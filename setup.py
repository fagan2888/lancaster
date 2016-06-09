from   setuptools import setup, Extension
import os
import os.path
import subprocess

def pkgconfig(*packages, **kw):
    flag_map = {'-I': 'include_dirs', '-L': 'library_dirs', '-l': 'libraries'}
    cmd = ['pkg-config', '--libs', '--cflags']
    cmd.extend(packages)
    for token in subprocess.check_output(cmd, stderr=open(os.devnull, 'w')).decode().split():
        if token[:2] in flag_map:
            kw.setdefault(flag_map.get(token[:2]), []).append(token[2:])
        elif token == '-pthread':
            kw.setdefault('extra_link_args', []).append(token)
        else:
            kw.setdefault('extra_compile_args', []).append(token)
    return kw

try:
    extension_kwargs = pkgconfig('avro-c')
except subprocess.CalledProcessError:
    extension_kwargs = {'libraries': ['avro', 'z', 'lzma', 'snappy']}

setup(name        = 'Lancaster',
      version     = '0.2.1',
      description = 'A python extension wrapper for avro-c',
      packages    = ['lancaster'],
      ext_modules = [Extension('lancaster._lancaster',
                               sources = ['lancaster/_lancaster.c'],
                               **extension_kwargs)])
