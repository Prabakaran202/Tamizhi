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

# 3. பழையதை அழித்துவிட்டு (DELETE) புதியதை ரீப்ளேஸ் செய்தல்
echo -e "\033[1;34m[3/4] Removing old global compiler & Installing new one...\033[0m"
if [ -d "/data/data/com.termux/files/usr/bin" ]; then
    # ஆண்ட்ராய்டு டெர்மக்ஸ்
    echo -e "\033[1;33m  -> Deleting old Tamizhi from Termux...\033[0m"
    rm -f /data/data/com.termux/files/usr/bin/tamizhi
    
    cp tamizhi /data/data/com.termux/files/usr/bin/tamizhi
    chmod +x /data/data/com.termux/files/usr/bin/tamizhi
    echo -e "\033[1;32m  -> New Tamizhi safely installed in Termux!\033[0m"
else
    # லினக்ஸ் சிஸ்டம் (லேப்டாப்)
    echo -e "\033[1;33m  -> Deleting old Tamizhi from Linux (/usr/local/bin)...\033[0m"
    sudo rm -f /usr/local/bin/tamizhi
    
    sudo cp tamizhi /usr/local/bin/tamizhi
    sudo chmod +x /usr/local/bin/tamizhi
    echo -e "\033[1;32m  -> New Tamizhi safely installed in Linux!\033[0m"
fi

# 4. தற்காலிக குப்பைகளைச் சுத்தம் செய்தல்
echo -e "\033[1;34m[4/4] Cleaning up temporary build artifacts...\033[0m"
rm -f output.bc output.ll

echo -e "\033[1;36m--------------------------------------------------\033[0m"
echo -e "\033[1;32m🏆 Success! Clean Installation Completed.\033[0m"
echo -e "\033[1;35m👉 Try running: tamizhi\033[0m"
echo -e "\033[1;36m--------------------------------------------------\033[0m"
