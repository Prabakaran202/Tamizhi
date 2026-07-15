# =====================================================================
# 🌟 தமிழி யுனிவர்சல் ஆண்ட்ராய்டு / லினக்ஸ் மாஸ்டர் மேக்ஃபைல் (v0.1.5 Stable)
# =====================================================================

# 1. கம்பைலர் தேர்வு: பெர்மிஷன் பிளாக் செய்யும் gcc-க்கு பதிலாக நேடிவ் 'clang' பயன்படுத்துகிறோம்
CC = clang
CFLAGS = -Iinclude -Wall -Wno-unused-function

# 2. டெர்மக்ஸ் எல்எல்விஎம் லைப்ரரி ஆப்செட் மேப்பிங் பிக்ஸ்
LLVM_FLAGS = -lLLVM

# 3. சோர்ஸ் கோப்புகள் (TTr நீக்கப்பட்டது, பியூர் தமிழி மட்டும்)
SRC_DIR = src
CODEGEN_DIR = src/codegen
CORE_DIR = core

# ஆட்டோமேட்டிக்காக அனைத்து ஃபோல்டர்களில் இருக்கும் .c ஃபைல்களையும் ஸ்கேன் செய்து எடுத்தல்
SRCS = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(CODEGEN_DIR)/*.c) $(wildcard $(CORE_DIR)/*.c)
TARGET = tamizhi

# டிஃபால்ட் கமெண்ட்: 'make' அடிச்சா இது ரன் ஆகும்
all: setup $(TARGET)

# 1. தேவையான ஸ்டோரேஜ் மற்றும் கேச் ஃபோல்டர்களை உருவாக்குதல்
setup:
	@mkdir -p storage
	@echo "[System] Storage infrastructure initialized safely."

# 2. தமிழி மெயின் இன்ஜினை பில்ட் செய்தல்
$(TARGET): $(SRCS)
	@echo "[Building] Orchestrating Tamizhi Modules with Native Clang Toolchain..."
	$(CC) $(CFLAGS) $(SRCS) $(LLVM_FLAGS) -o $(TARGET)
	@echo -e "\033[1;36m==================================================\033[0m"
	@echo -e "\033[1;32m   🚀 TAMIZHI UNIVERSAL SHELL ENGINE READY! 🔥    \033[0m"
	@echo -e "\033[1;36m==================================================\033[0m"

# பழைய டெம்ப்ரவரி ஃபைல்களை நீக்க
clean:
	@echo "[Cleaning] Removing binaries and temporary files..."
	@rm -f $(TARGET)
	@rm -rf storage/*.dna
