PROJECT    := cat-fountain
MCU        := atmega328p
F_CPU      := 8000000UL

CXX        := avr-g++
OBJCOPY    := avr-objcopy
SIZE       := avr-size

BUILD_DIR  ?= build
SRC_DIR    ?= src
INC_DIR    ?= include

TARGET     := $(BUILD_DIR)/$(PROJECT)
ELF        := $(TARGET).elf
HEX        := $(TARGET).hex

SOURCES    := $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS    := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
DEPENDS    := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.d,$(SOURCES))

CPPFLAGS   := -I$(INC_DIR) -DF_CPU=$(F_CPU) -MMD -MP
CXXFLAGS   := \
	-mmcu=$(MCU) \
	-std=gnu++17 \
	-Os \
	-Wall -Wextra -Wpedantic \
	-ffunction-sections \
	-fdata-sections \
	-fno-exceptions \
	-fno-rtti \
	-fno-threadsafe-statics

LDFLAGS    := -mmcu=$(MCU) -Wl,--gc-sections

-include $(DEPENDS)

.PHONY: all clean

all: $(HEX)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Compilar desde el directorio raíz, sin cambiar de directorio
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(ELF): $(OBJECTS)
	$(CXX) $(OBJECTS) $(LDFLAGS) -o $@
	$(SIZE) $@

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex -R .eeprom $< $@

clean:
	rm -rf $(BUILD_DIR)/*