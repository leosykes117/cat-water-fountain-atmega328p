# =============================================================================
# Firmware Makefile — runs INSIDE the Docker container
# =============================================================================

MCU       := atmega328p
F_CPU     := 8000000UL

CXX       := avr-g++
OBJCOPY   := avr-objcopy
SIZE      := avr-size

TARGET    := cat-fountain
BUILD_DIR ?= build

SRC       := src/main.cpp

CXXFLAGS  := \
    -mmcu=$(MCU) \
    -DF_CPU=$(F_CPU) \
    -Os \
    -Wall -Wextra \
    -std=c++17 \
    -ffunction-sections \
    -fdata-sections \
    -fno-exceptions \
    -fno-rtti \
    -I include

LDFLAGS   := -mmcu=$(MCU) -Wl,--gc-sections

ELF       := $(BUILD_DIR)/$(TARGET).elf
HEX       := $(BUILD_DIR)/$(TARGET).hex

# -----------------------------------------------------------------------------
.PHONY: all clean

all: $(HEX)
	@$(SIZE) $(ELF)

$(ELF): $(SRC)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -f $(ELF) $(HEX)