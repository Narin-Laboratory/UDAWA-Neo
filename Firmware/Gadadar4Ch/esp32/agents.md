# Exit immediately if a command exits with a non-zero status.
set -e

echo "--- Installing PlatformIO Core ---"
# Install PIO Core and add it to the PATH for the current session
pip install platformio
export PATH="$HOME/.local/bin:$PATH"

echo "--- Installing Espressif 32 Platform ---"
pio platform install espressif32

echo "--- Preparing Project Files in Subdirectory ---"

# 1. Navigate into the deep subdirectory where the PIO project lives
# (Jules runs this script from the repository root: /app)
cd Firmware/Gadadar4Ch/esp32

# 2. Rename the configuration file from .sample to the standard name
cp platformio.ini.sample platformio.ini

# 3. Create the required 'secret.h' file in the 'include' directory
# (Assuming 'include' is relative to the current directory: Firmware/Gadadar4Ch/esp32)
cp include/secret.h-example include/secret.h

echo "--- Running Full Project Build for Gadadar4Ch ---"

# 4. Run the PlatformIO build command. We need to explicitly target 
# the environment named 'Gadadar4Ch' which is specified in your platformio.ini.
# pio run --environment Gadadar4Ch

echo "--- Environment Setup Complete ---"