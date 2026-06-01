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

# 🌟 [MASTER FIX]: GitHub-ல் இருந்து டவுன்லோட் செய்யாமல், 
# நாம் லோக்கல்ல திருத்திய புது கோப்பையே (Local Workspace) நேரடியாக பில்ட் செய்கிறோம்!
echo "[1/4] Validating local source code environment..."
if [ ! -f "src/main.c" ] || [ ! -f "src/parser.c" ]; then
    echo "பிழை: சோர்ஸ் கோடுகள் (src/) தற்போதைய ஃபோல்டரில் இல்லை!"
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

# 4. தற்காலிக குப்பைகளைச் சுத்தம் செய்தல்
echo "[4/4] Cleaning up temporary build artifacts..."
rm -f output.bc output.ll

echo "--------------------------------------------------"
echo "🏆 Success! Tamizhi Compiler is installed globally."
echo "👉 Try running: tamizhi"
echo "--------------------------------------------------"
