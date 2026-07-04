#!/bin/bash

echo -e "\033[1;36m--------------------------------------------------\033[0m"
echo -e "\033[1;33m  🍁 Tamizhi Compiler Global Installer Started 🍁\033[0m"
echo -e "\033[1;36m--------------------------------------------------\033[0m"

# 0. லோக்கல் ஒர்க்ஸ்பேஸை சுத்தம் செய்தல்
if [ -f "Makefile" ]; then
    echo -e "\033[1;34m[0/4] Cleaning up local workspace to avoid conflicts...\033[0m"
    make clean > /dev/null 2>&1
    rm -f tamizhi output.bc output.ll
fi

# 1. சோர்ஸ் கோடு சரிபார்ப்பு
echo -e "\033[1;34m[1/4] Validating local source code environment...\033[0m"
if [ ! -f "main.c" ] && [ ! -f "src/main.c" ]; then
    echo -e "\033[1;31mபிழை: சோர்ஸ் கோடுகள் தற்போதைய ஃபோல்டரில் இல்லை!\033[0m"
    exit 1
fi

# 2. புதிய கம்பைலரை பில்ட் செய்தல்
echo -e "\033[1;34m[2/4] Building Tamizhi Engine using Clang Toolchain...\033[0m"
make clean && make

if [ ! -f "tamizhi" ]; then
    echo -e "\033[1;31mபிழை: தமிழி இன்ஜின் பில்ட் செய்ய முடியவில்லை! C கோடை சரிபார்க்கவும்.\033[0m"
    exit 1
fi

# 3. பைனரியை Python CLI தேடும் இடத்திற்கு நகர்த்துதல் (Crucial Step)
echo -e "\033[1;34m[3/4] Installing Tamizhi to the exact location requested by PIP Workflow...\033[0m"

# ~/.tamizhi/bin மற்றும் ~/.tamizhi/core ஃபோல்டர்களை உருவாக்குதல்
mkdir -p ~/.tamizhi/bin
mkdir -p ~/.tamizhi/core

# பழைய பைனரியை அழித்தல்
rm -f ~/.tamizhi/bin/tamizhi

# புதிதாக கம்பைல் செய்த பைனரியை நகர்த்துதல்
cp tamizhi ~/.tamizhi/bin/tamizhi
chmod +x ~/.tamizhi/bin/tamizhi

# கம்பைலருக்குத் தேவையான core ஃபைல்களையும் பாதுகாப்பாக நகர்த்துதல்
if [ -d "core" ]; then
    cp -r core/* ~/.tamizhi/core/
fi
echo -e "\033[1;32m  -> New Tamizhi safely installed in ~/.tamizhi/bin/ \033[0m"

# 4. தற்காலிக குப்பைகளைச் சுத்தம் செய்தல்
echo -e "\033[1;34m[4/4] Cleaning up temporary build artifacts...\033[0m"
rm -f output.bc output.ll

echo -e "\033[1;36m--------------------------------------------------\033[0m"
echo -e "\033[1;32m🏆 Success! Clean Installation Completed.\033[0m"
echo -e "\033[1;35m👉 Try running: tamizhi\033[0m"
echo -e "\033[1;36m--------------------------------------------------\033[0m"
