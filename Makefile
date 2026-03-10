
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200112L 
LDFLAGS := -lcurl -ljansson -lpthread
BUILD   := build

# Add include directories
CFLAGS  += -ILibs \
           -ILibs/Utils \
           -IServer \
		   -IServer/Connection \
		   -IServer/Log \
		   -ILibs/Algorithm \


# ============================================
# Source files
# ============================================
SRC := $(shell find Libs Server -name "*.c")
OBJ := $(patsubst %.c, $(BUILD)/%.o, $(SRC))

# ---- Final target ----
PREFIX ?= /usr/local/bin
TARGET := Glennergy-Main

INSTALLDIR = $(PREFIX)

all: $(TARGET)

$(TARGET): $(OBJ)
	@echo "Linking $@..."	
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $@"

# ---- Rule to compile each .c file ----
$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

modules-all: $(TARGET)
	@echo "Building all submodules..."
	@$(MAKE) -C API/Meteo
	@$(MAKE) -C API/Spotpris
	@$(MAKE) -C Cache
	@$(MAKE) -C Algorithm
	@echo "All modules built successfully!"

modules: modules-all

# ============================================
# Debug build
# ============================================

DEBUG_BUILD := build_debug
DEBUG_TARGET := gln_app_debug
DEBUG_FLAGS := -g -O0 -DDEBUG

debug: CFLAGS += $(DEBUG_FLAGS)
debug: BUILD := $(DEBUG_BUILD)
debug: OBJ := $(patsubst %.c, $(DEBUG_BUILD)/%.o, $(SRC))
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(OBJ)
	@echo "Linking $@(debug)..."	
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Debug build complete: $@"

$(DEBUG_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@ 


install:
	# Create directory
	install -d $(DESTDIR)$(INSTALLDIR)

	# Install binary
	install -m 755 $(TARGET) $(DESTDIR)$(INSTALLDIR)

install-all: modules-all
	@echo "Installing all modules..."
	@$(MAKE) install
	@$(MAKE) -C API/Meteo install
	@$(MAKE) -C API/Spotpris install
	@$(MAKE) -C Cache install
	@$(MAKE) -C Algorithm install
	@echo "All modules installed!"

uninstall:
	rm -rf $(DESTDIR)$(INSTALLDIR)/$(TARGET)

uninstall-all:
	@echo "Uninstalling all modules..."
	@$(MAKE) uninstall
	@$(MAKE) -C API/Meteo uninstall
	@$(MAKE) -C API/Spotpris uninstall
	@$(MAKE) -C Cache uninstall
	@$(MAKE) -C Algorithm uninstall
	@echo "All modules uninstalled!"

# ---- Cleanup ----
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD) $(DEBUG_BUILD)
	rm -f $(TARGET) $(DEBUG_TARGET)
	@echo "Clean complete"

clean-all:
	@echo "Cleaning all modules..."
	@$(MAKE) clean
	@$(MAKE) -C API/Meteo clean
	@$(MAKE) -C API/Spotpris clean
	@$(MAKE) -C Cache clean
	@$(MAKE) -C Algorithm clean
	@echo "All modules cleaned!"

# ============================================
# Help
# ============================================



.PHONY: all clean debug install uninstall dirs run test help
