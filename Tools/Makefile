# make        # Build both
# make x64    # Build 64-bit only
# make x86    # Build 32-bit only
# make clean  # Clean build files

# Compilers
CC_X64 = x86_64-w64-mingw32-gcc
CC_X86 = i686-w64-mingw32-gcc

# Flags
CFLAGS = -O2 -Wall -masm=intel -mwindows
LDFLAGS = -shared -s

# Directories
OUT_DIR_X64 = x64
OUT_DIR_X86 = x86

# Target names
TARGET_X64 = $(OUT_DIR_X64)/SampleDLL.dll
TARGET_X86 = $(OUT_DIR_X86)/SampleDLL.dll

# Source files
SRC = SampleDLL.cpp

all: dirs x64 x86

dirs:
	mkdir -p $(OUT_DIR_X64)
	mkdir -p $(OUT_DIR_X86)

x64: $(TARGET_X64)

x86: $(TARGET_X86)

$(TARGET_X64): $(SRC)
	$(CC_X64) $(CFLAGS) $(LDFLAGS) -o $@ $

$(TARGET_X86): $(SRC)
	$(CC_X86) $(CFLAGS) $(LDFLAGS) -o $@ $

clean:
	rm -rf $(OUT_DIR_X64) $(OUT_DIR_X86)

.PHONY: all dirs x64 x86 clean
