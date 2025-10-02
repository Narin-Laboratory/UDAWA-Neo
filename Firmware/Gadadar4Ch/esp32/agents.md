# How to setup build environment
set -e
pip install platformio
cp platformio.ini.sample platformio.ini
cp include/secret.h-example include/secret.h
cd Firmware/Gadadar4Ch/esp32
pio pkg install -e Gadadar4Ch