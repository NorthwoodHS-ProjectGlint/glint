
# build os library
mkdir -p -m 777 _build && cd _build
cmake ..
sudo make install --silent
cd ../

cp examples/app-template/*.glt _device/titles/

# start bootloader
mkdir -p -m 777 _device && cd _device
../_build/system/bootloader/bootloader