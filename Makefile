# =====================================================================
# 🌟 தமிழி யுனிவர்சல் ஆண்ட்ராய்டு / லினக்ஸ் மாஸ்டர் மேக்ஃபைல் (Stable Build)
# =====================================================================

# 1. கம்பைலர் தேர்வு: பெர்மிஷன் பிளாக் செய்யும் gcc-க்கு பதிலாக நேடிவ் 'clang' பயன்படுத்துகிறோம்
CC = clang
CFLAGS = -Iinclude -Wall -Wno-unused-function

# 2. டெர்மக்ஸ் எல்எல்விஎம் லைப்ரரி ஆப்செட் மேப்பிங் பிக்ஸ்
# 'llvm-config' பெர்மிஷன் எரர் தருவதால், அதற்கு பதிலாக நேரடி எல்எல்விஎம் லிங்கிங் ஃபிளாக்ஸ்
LLVM_FLAGS = -lLLVM

# 3. சோர்ஸ் கோப்புகள் (Source Files Layout)
SRCS = src/main.c src/lexer.c src/parser.c src/codegen.c core/encoder.c core/decoder.c
TARGET = tamizhi

# டிஃபால்ட் கமெண்ட்: 'make' அடிச்சா இது ரன் ஆகும்
all: setup $(TARGET)

# 1. தேவையான ஸ்டோரேஜ் ஃபோல்டர்களை உருவாக்குதல்
setup:
	@mkdir -p storage
	@echo "[System] Storage folder initialized."

# 2. தமிழி மெயின் இன்ஜினை பில்ட் செய்தல் (No llvm-config or gcc permission dependency)
$(TARGET): $(SRCS)
	@echo "[Building] Tamizhi Engine with Native Clang Toolchain..."
	$(CC) $(CFLAGS) $(SRCS) $(LLVM_FLAGS) -o $(TARGET)
	@echo "[Success] Tamizhi is ready to use!"

# பழைய டெம்ப்ரவரி ஃபைல்களை நீக்க
clean:
	@echo "[Cleaning] Removing binaries and temporary files..."
	@rm -f $(TARGET)
	@rm -rf storage/*.dna
