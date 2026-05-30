#!/bin/bash

echo "--------------------------------------------------"
echo "🍁 Tamizhi Compiler Global Installer Started 🍁"
echo "--------------------------------------------------"

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

# ஒருவேளை பில்ட் தோல்வியுற்றால் இன்ஸ்டாலேஷனை நிறுத்த
if [ ! -f "tamizhi" ]; then
    echo "பிழை: தமிழி இன்ஜின் பில்ட் செய்ய முடியவில்லை!"
    exit 1
