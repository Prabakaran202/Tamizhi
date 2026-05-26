#!/bin/bash

echo "--------------------------------------------------"
echo "🍁 Tamizhi Compiler Global Installer Started 🍁"
echo "--------------------------------------------------"

# 🌟 Termux மற்றும் Linux இரண்டிற்கும் செட் ஆகும் பாதுகாப்பான டெம்ப் பாத்
BUILD_DIR="${TMPDIR:-/tmp}/tamizhi_build"

# 1. தற்காலிக ஃபோல்டரில் ரெப்போவை க்ளோன் செய்தல்
echo "[1/4] Downloading source codes from GitHub..."
rm -rf "$BUILD_DIR"
git clone --depth=1 https://github.com/Prabakaran2002/Tamizhi.git "$BUILD_DIR"

cd "$BUILD_DIR" || exit 1

# 2. கம்பைலரை பில்ட் செய்தல்
echo "[2/4] Building Tamizhi Engine using Clang Toolchain..."
make clean && make

# 3. சிஸ்டம் பாத் கண்டறிந்து இன்ஸ்டால் செய்தல்
echo "[3/4] Installing binary to system path..."
if [ -d "/data/data/com.termux/files/usr/bin" ]; then
    # ஆண்ட்ராய்டு டெர்மக்ஸ் ஆக இருந்தால்
    cp tamizhi /data/data/com.termux/files/usr/bin/tamizhi
    chmod +x /data/data/com.termux/files/usr/bin/tamizhi
else
    # லினக்ஸ் லேப்டாப் ஆக இருந்தால்
    sudo cp tamizhi /usr/local/bin/tamizhi
    sudo chmod +x /usr/local/bin/tamizhi
fi

# 4. தற்காலிக ஃபைல்களை நீக்குதல்
cd ~ || exit
rm -rf "$BUILD_DIR"

echo "--------------------------------------------------"
echo "🏆 Success! Tamizhi Compiler is installed globally."
echo "👉 Try running: tamizhi"
echo "--------------------------------------------------"
