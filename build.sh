
# build os library
mkdir -p -m 777 _build && cd _build
cmake ..
make install --silent
cd ../

# build example
cd _build
cmake ../examples/app-template
make
cd ../
cp examples/app-template/*.glt _device/titles/

# start bootloader
mkdir -p -m 777 _device && cd _device
../_build/system/bootloader/bootloader