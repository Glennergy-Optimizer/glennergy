CC      := gcc
CFLAGS  := -g -O0 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200112L
LDFLAGS := -lcurl -ljansson -lpthread
BUILD   := build

# Production components are intentionally listed here. Do not discover
# Makefiles dynamically: adding a test or legacy Makefile must not change the
# contents of a production build.
PRODUCTION_COMPONENT_DIRS := \
	Cache \
	Algorithm \
	API/Meteocpp \
	API/Spotpris

# Add include directories
CFLAGS += -ILibs \
	-ILibs/Utils \
	-IServer \
	-IServer/Connection \
	-IServer/Log \
	-IServer/HTTP

# Source files
SRC := $(shell find Libs Server -name "*.c")
OBJ := $(patsubst %.c,$(BUILD)/%.o,$(SRC))

# Main server target
PREFIX ?= /usr/local/bin
TARGET := Glennergy-Main
INSTALLDIR = $(PREFIX)

all: $(TARGET) production-components

production-components:
	@set -e; \
	for dir in $(PRODUCTION_COMPONENT_DIRS); do \
		echo "Building production component: $$dir"; \
		$(MAKE) -C "$$dir" all; \
	done

$(TARGET): $(OBJ)
	@echo "Linking $@..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Build complete: $@"

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Debug build of the main server. Component-specific debug builds can be
# introduced once their Makefiles expose matching debug targets.
DEBUG_BUILD := build_debug
DEBUG_TARGET := gln_app_debug
DEBUG_FLAGS := -g -O0 -DDEBUG
DEBUG_OBJ := $(patsubst %.c,$(DEBUG_BUILD)/%.o,$(SRC))

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): CFLAGS += $(DEBUG_FLAGS)
$(DEBUG_TARGET): $(DEBUG_OBJ)
	@echo "Linking $@ (debug)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Debug build complete: $@"

$(DEBUG_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Install lifecycle orchestration is intentionally handled in migration stage
# 3. These targets currently retain the existing main-binary behavior.
install:
	install -d $(DESTDIR)$(INSTALLDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(INSTALLDIR)

uninstall:
	rm -f $(DESTDIR)$(INSTALLDIR)/$(TARGET)

clean:
	@echo "Cleaning build files..."
	rm -rf $(BUILD) $(DEBUG_BUILD)
	rm -f $(TARGET) $(DEBUG_TARGET)
	@set -e; \
	for dir in $(PRODUCTION_COMPONENT_DIRS); do \
		echo "Cleaning production component: $$dir"; \
		$(MAKE) -C "$$dir" clean; \
	done
	@echo "Clean complete"

.PHONY: all production-components clean debug install uninstall
