# =============================================================================
# cat-fountain — top-level Makefile
#
# Targets:
#   make build    — build the Docker image
#   make compile  — compile the firmware inside the container
#   make flash    — flash the .hex to the ATmega328P via avrdude (runs locally)
#   make clean    — remove build artifacts
# =============================================================================

IMAGE      := avr:latest
BUILD_DIR  := build
ROOT       := $(abspath .)

# avrdude settings (runs on host, not inside Docker)
PROGRAMMER := usbasp
MCU        := atmega328p
PORT       ?= usb

# Derived
HEX_FILE   := $(BUILD_DIR)/cat-fountain.hex

ADDIONAL_FLAGS ?=

# -----------------------------------------------------------------------------
.PHONY: build compile flash clean

## Build the Docker image
build:
	docker build -t $(IMAGE) $(ROOT)/docker

## Compile firmware inside the container
compile:
	@mkdir -p $(BUILD_DIR)
	docker run --rm \
		--mount type=bind,src=$(ROOT)/src,dst=/work/src,ro \
		--mount type=bind,src=$(ROOT)/include,dst=/work/include,ro \
		--mount type=bind,src=$(ROOT)/$(BUILD_DIR),dst=/work/build \
		-w /work \
		$(IMAGE) sh -lc '\
		make -f /work/src/Makefile \
			ADDIONAL_FLAGS="$(ADDIONAL_FLAGS)" \
			BUILD_DIR=/work/build \
			SRC_DIR=/work/src \
			INC_DIR=/work/include all \
		&& echo "" && echo "=== Artifacts ===" && \
		find /work/build -maxdepth 1 -type f \( -name "*.hex" -o -name "*.elf" \) -print \
		'

## Flash firmware to MCU using avrdude (runs on host)
flash: compile
	avrdude \
		-c $(PROGRAMMER) \
		-p $(MCU) \
		-P $(PORT) \
		-U flash:w:$(HEX_FILE):i

## Remove build artifacts
clean:
	rm -rf $(BUILD_DIR)/*.o
