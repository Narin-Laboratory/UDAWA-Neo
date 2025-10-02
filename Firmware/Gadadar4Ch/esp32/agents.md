# How to setup build environment
set -e
pip install platformio
export PATH="$HOME/.local/bin:$PATH"
pio pkg install espressif32
cd Firmware/Gadadar4Ch/esp32
cp platformio.ini.sample platformio.ini
cp include/secret.h-example include/secret.h