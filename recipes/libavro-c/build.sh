mkdir build
cd build
cmake -D CMAKE_INSTALL_PREFIX=$PREFIX \
      -D SNAPPY_ROOT_DIR=$PREFIX \
      -D CMAKE_BUILD_TYPE=RelWithDebInfo \
      ..
make
make test
make install
