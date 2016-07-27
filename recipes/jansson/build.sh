mkdir build
cd build
cmake -D CMAKE_INSTALL_PREFIX=$PREFIX \
      -D CMAKE_BUILD_TYPE=RelWithDebInfo \
      -D JANSSON_BUILD_SHARED_LIBS=1 \
      -D JANSSON_BUILD_DOCS=OFF \
      ..
make
make test
make install
