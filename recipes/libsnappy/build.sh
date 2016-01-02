#!/bin/bash

autoreconf -fvi
./configure --prefix=$PREFIX
make
make install
