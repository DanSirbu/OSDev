rm -rf build/
mkdir -p build
cd build

cmake ../ $1
make