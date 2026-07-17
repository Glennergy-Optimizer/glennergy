CC      := gcc
CFLAGS  := -g -O0 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200112L
DEPFLAGS := -MMD -MP
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
DEP := $(OBJ:.o=.d)

# Main server target
PREFIX ?= /usr/local
TARGET := Glennergy-Main

# Installation layout. Runtime directories are created by systemd in a later
# migration stage; make install owns only versioned/static files.
LIBEXECDIR ?= $(PREFIX)/libexec/glennergy
DATADIR ?= $(PREFIX)/share/glennergy
SYSCONFDIR ?= /etc/glennergy
LOCALSTATEDIR ?= /var
STATEDIR ?= $(LOCALSTATEDIR)/lib/glennergy
CACHEDIR ?= $(LOCALSTATEDIR)/cache/glennergy
UNITDIR ?= /etc/systemd/system

CONFIG_SOURCE := API/Glennergy-Fastigheter.json
CONFIG_FILE := $(SYSCONFDIR)/fastigheter.json
CONFIG_EXAMPLE := $(DATADIR)/fastigheter.example.json

PRODUCTION_BINARIES := \
	$(TARGET) \
	Cache/Glennergy-InputCache \
	Algorithm/Glennergy-Algoritm \
	API/Meteocpp/Glennergy-Meteo \
	API/Spotpris/Glennergy-Spotpris

SYSTEMD_UNITS := \
	systemd/glennergy.target \
	systemd/glennergy-inputcache.service \
	systemd/glennergy-algorithm.service \
	systemd/glennergy-server.service \
	systemd/glennergy-meteo.service \
	systemd/glennergy-meteo.timer \
	systemd/glennergy-spotpris.service \
	systemd/glennergy-spotpris.timer

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
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

# Debug build of the main server. Component-specific debug builds can be
# introduced once their Makefiles expose matching debug targets.
DEBUG_BUILD := build_debug
DEBUG_TARGET := gln_app_debug
DEBUG_FLAGS := -g -O0 -DDEBUG
DEBUG_OBJ := $(patsubst %.c,$(DEBUG_BUILD)/%.o,$(SRC))
DEBUG_DEP := $(DEBUG_OBJ:.o=.d)

debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): CFLAGS += $(DEBUG_FLAGS)
$(DEBUG_TARGET): $(DEBUG_OBJ)
	@echo "Linking $@ (debug)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Debug build complete: $@"

$(DEBUG_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEP) $(DEBUG_DEP)

check-install-inputs:
	@set -e; \
	for binary in $(PRODUCTION_BINARIES); do \
		if [ ! -s "$$binary" ]; then \
			echo "Missing or empty build artifact: $$binary" >&2; \
			echo "Build and validate the release before running make install." >&2; \
			exit 1; \
		fi; \
	done; \
	if [ ! -s "$(CONFIG_SOURCE)" ]; then \
		echo "Missing or empty configuration source: $(CONFIG_SOURCE)" >&2; \
		exit 1; \
	fi; \
	for unit in $(SYSTEMD_UNITS); do \
		if [ ! -s "$$unit" ]; then \
			echo "Missing or empty systemd unit: $$unit" >&2; \
			exit 1; \
		fi; \
	done
	@echo "Install inputs are complete."

install: check-install-inputs
	install -d -m 0755 "$(DESTDIR)$(LIBEXECDIR)"
	@set -e; \
	for binary in $(PRODUCTION_BINARIES); do \
		install -m 0755 "$$binary" "$(DESTDIR)$(LIBEXECDIR)/"; \
	done
	install -d -m 0755 "$(DESTDIR)$(DATADIR)"
	install -m 0644 "$(CONFIG_SOURCE)" "$(DESTDIR)$(CONFIG_EXAMPLE)"
	install -d -m 0755 "$(DESTDIR)$(SYSCONFDIR)"
	@if [ ! -e "$(DESTDIR)$(CONFIG_FILE)" ]; then \
		install -m 0600 "$(CONFIG_SOURCE)" "$(DESTDIR)$(CONFIG_FILE)"; \
		echo "Installed initial configuration: $(CONFIG_FILE)"; \
	else \
		echo "Preserving existing configuration: $(CONFIG_FILE)"; \
	fi
	install -d -m 0755 "$(DESTDIR)$(UNITDIR)"
	@set -e; \
	for unit in $(SYSTEMD_UNITS); do \
		install -m 0644 "$$unit" "$(DESTDIR)$(UNITDIR)/"; \
	done

uninstall:
	@set -e; \
	for binary in $(PRODUCTION_BINARIES); do \
		rm -f "$(DESTDIR)$(LIBEXECDIR)/$$(basename "$$binary")"; \
	done
	rm -f "$(DESTDIR)$(CONFIG_EXAMPLE)"
	@set -e; \
	for unit in $(SYSTEMD_UNITS); do \
		rm -f "$(DESTDIR)$(UNITDIR)/$$(basename "$$unit")"; \
	done
	-rmdir "$(DESTDIR)$(LIBEXECDIR)" 2>/dev/null
	-rmdir "$(DESTDIR)$(DATADIR)" 2>/dev/null
	@echo "Preserved configuration: $(CONFIG_FILE)"

purge:
	@if [ "$(CONFIRM_PURGE)" != "YES" ]; then \
		echo "Refusing to remove Glennergy configuration and data." >&2; \
		echo "Run with CONFIRM_PURGE=YES only after backing up required data." >&2; \
		exit 2; \
	fi
	@set -e; \
	for path in "$(SYSCONFDIR)" "$(STATEDIR)" "$(CACHEDIR)"; do \
		case "$$path" in \
			""|/) echo "Refusing unsafe purge path: '$$path'" >&2; exit 2 ;; \
		esac; \
	done
	$(MAKE) uninstall
	rm -rf -- "$(DESTDIR)$(SYSCONFDIR)"
	rm -rf -- "$(DESTDIR)$(STATEDIR)"
	rm -rf -- "$(DESTDIR)$(CACHEDIR)"
	@echo "Removed Glennergy configuration, persistent state, and cache."

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

.PHONY: all production-components clean debug check-install-inputs install uninstall purge
