# How to setup build environment
set -e
cd Firmware/Gadadar4Ch/esp32
cp platformio.ini.sample platformio.ini
cp include/secret.h-example include/secret.h
pip install platformio
pio pkg install -e Gadadar4Ch