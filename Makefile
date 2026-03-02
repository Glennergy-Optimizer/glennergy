
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


uninstall:
	rm -rf $(DESTDIR)$(INSTALLDIR)/$(TARGET)


# ---- Cleanup ----
clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD) $(DEBUG_BUILD)
	rm -f $(TARGET) $(DEBUG_TARGET)
	@echo "Clean complete"

# ============================================
# Help
# ============================================



.PHONY: all clean debug install uninstall dirs run test help
