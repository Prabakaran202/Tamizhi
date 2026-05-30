#!/bin/bash

echo "--------------------------------------------------"
echo "🍁 Tamizhi Compiler Global Installer Started 🍁"
echo "--------------------------------------------------"

# 🌟 லோக்கல் ஃபைல் கன்ஃப்ளிக்ட்டைத் தவிர்க்க தற்போதைய ஃபோல்டரை சுத்தம் செய்கிறோம்
if [ -f "Makefile" ]; then
    echo "[0/4] Cleaning up local workspace to avoid conflicts..."
    make clean > /dev/null 2>&1
    rm -f tamizhi output.bc output.ll
fi

# 🌟 புது பிக்ஸ்: Termux-ல் எரர் வராத பாதுகாப்பான ஹோம் டெம்ப் பாத் 🚀
BUILD_DIR="$HOME/tamizhi_build"

# பழைய பில்ட் ஃபோல்டர் இருந்தால் அதை முதலில் சுத்தம் செய்கிறோம்
rm -rf "$BUILD_DIR"

# 1. தற்காலிக ஃபோல்டரில் ரெப்போவை க்ளோன் செய்தல்
echo "[1/4] Downloading source codes from GitHub..."
git clone --depth=1 https://github.com/Prabakaran202/Tamizhi.git "$BUILD_DIR"

# 🌟 முக்கிய பிக்ஸ்: க்ளோன் செய்யப்பட்ட தமிழி ஃபோல்டருக்குள் நுழைகிறோம்!
if [ -d "$BUILD_DIR" ]; then
    cd "$BUILD_DIR" || exit
else
    echo "பிழை: சோர்ஸ் கோடு டவுன்லோட் செய்வதில் சிக்கல்!"
    exit 1
fi

# 2. கம்பைலரை பில்ட் செய்தல்
echo "[2/4] Building Tamizhi Engine using Clang Toolchain..."
make clean && make

# ஒருவேளை பில்ட் தோல்வியுற்றால் இன்ஸ்டாலேஷனை நிறுத்த பாதுகாப்பு வளையம்
if [ ! -f "tamizhi" ]; then
    echo "பிழை: தமிழி இன்ஜின் பில்ட் செய்ய முடியவில்லை!"
    exit 1
fi

# 3. சிஸ்டம் பாத் கண்டறிந்து குளோபலாக இன்ஸ்டால் செய்தல்
echo "[3/4] Installing binary to system path..."
if [ -d "/data/data/com.termux/files/usr/bin" ]; then
    # ஆண்ட்ராய்டு டெர்மக்ஸ் சூழலாக இருந்தால்
    cp tamizhi /data/data/com.termux/files/usr/bin/tamizhi
    chmod +x /data/data/com.termux/files/usr/bin/tamizhi
else
    # லினக்ஸ் சிஸ்டம் (லேப்டாப்) ஆக இருந்தால்
    sudo cp tamizhi /usr/local/bin/tamizhi
    sudo chmod +x /usr/local/bin/tamizhi
fi

# 4. தற்காலிகமாக உருவாக்கப்பட்ட பில்ட் டைரக்டரியை நீக்குதல்
cd ~ || exit
rm -rf "$BUILD_DIR"

echo "--------------------------------------------------"
echo "🏆 Success! Tamizhi Compiler is installed globally."
echo "👉 Try running: tamizhi"
echo "--------------------------------------------------"
