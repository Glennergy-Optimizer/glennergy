
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L 
LDFLAGS := -lcurl -ljansson -lpthread
BUILD   := build

# Add include directories
CFLAGS  += -ILibs \
           -ILibs/Utils \
           -IServer \
		   -IServer/Connection \
		   -IServer/Log \
		   -ILibs/Algorithm


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
# Installation and uninstallation
# ============================================

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

# ============================================
# Cleanup
# ============================================

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

.PHONY: all clean debug install uninstall dirs run test help


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

# ============================================
# Profile build (for valgrind/cachegrind)
# ============================================

PROFILE_BUILD := build_profile
PROFILE_TARGET := glenn_profile_valgrind
PROFILE_FLAGS := -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -g -O0

# Build ALL profile binaries (main + modules)
profile: CFLAGS += $(PROFILE_FLAGS)
profile: BUILD := $(PROFILE_BUILD)
profile: OBJ := $(patsubst %.c, $(PROFILE_BUILD)/%.o, $(SRC))
profile: $(PROFILE_TARGET)
	@echo "Building modules in profile mode..."
	@$(MAKE) -C API/Meteo clean && $(MAKE) -C API/Meteo CFLAGS="$(PROFILE_FLAGS)"
	@$(MAKE) -C API/Spotpris clean && $(MAKE) -C API/Spotpris CFLAGS="$(PROFILE_FLAGS)"
	@$(MAKE) -C Cache clean && $(MAKE) -C Cache CFLAGS="$(PROFILE_FLAGS)"
	@$(MAKE) -C Algorithm clean && $(MAKE) -C Algorithm CFLAGS="$(PROFILE_FLAGS)"
	@echo "All profile builds complete!"

$(PROFILE_TARGET): $(OBJ)
	@echo "Linking $@ (profile)..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROFILE_BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Profile main server (catches forked children automatically)
run-profile-server: profile
	@echo "Profiling main server + forked children..."
	@sudo chown -R $(USER):$(USER) /var/log/glennergy /var/cache/glennergy /var/run/glennergy 2>/dev/null || true
	@sudo rm -f /dev/shm/glennergy_shm /tmp/fifo_* 2>/dev/null || true
	@echo "Press Ctrl+C when done"
	valgrind --tool=cachegrind \
			 --cache-sim=yes \
			 --trace-children=yes \
			 --cachegrind-out-file=cachegrind.server.%p \
			 ./$(PROFILE_TARGET)

# Profile Meteo module
run-profile-meteo: profile
	@echo "Profiling Meteo..."
	valgrind --tool=cachegrind \
			 --cache-sim=yes \
			 --cachegrind-out-file=cachegrind.meteo.%p \
			 ./API/Meteo/Glennergy-Meteo

# Profile Spotpris module
run-profile-spotpris: profile
	@echo "Profiling Spotpris..."
	valgrind --tool=cachegrind \
			 --cache-sim=yes \
			 --cachegrind-out-file=cachegrind.spotpris.%p \
			 ./API/Spotpris/Glennergy-Spotpris

run-callgrind-server: profile
	@echo "Profiling function calls..."
	@sudo chown -R $(USER):$(USER) /var/log/glennergy /var/cache/glennergy /var/run/glennergy 2>/dev/null || true
	@sudo rm -f /dev/shm/glennergy_shm /tmp/fifo_* 2>/dev/null || true
	@echo "Press Ctrl+C when done"
	valgrind --tool=callgrind \
			 --trace-children=yes \
			 --callgrind-out-file=callgrind.server.%p \
			 ./$(PROFILE_TARGET)

# Analyze function calls
analyze-callgrind:
	@echo "================================================"
	@echo "  FUNCTION CALL ANALYSIS - ALL PROCESSES"
	@echo "================================================"
	@for f in callgrind.server.*; do \
		CMD=$$(head -10 "$$f" | grep "cmd:" | cut -d: -f2); \
		echo ""; \
		echo "--- $$f ($$CMD) ---"; \
		callgrind_annotate --auto=yes "$$f" 2>/dev/null | \
			grep -iE 'glennergy|Server|Cache|Algorithm|Meteo|Spotpris|Input|solar|optimizer' | \
			head -15; \
	done

# Analyze results
analyze-profile:
	@for file in cachegrind.*.* ; do \
		if [ -f "$$file" ]; then \
			echo ""; \
			echo "--- $$file ---"; \
			cg_annotate --auto=yes $$file 2>/dev/null | \
				grep -iE 'PROGRAM TOTALS|Server|Cache|Algorithm|Meteo|Spotpris' | head -10; \
		fi; \
	done

# Clean
clean-profile:
	rm -f cachegrind.*.* $(PROFILE_TARGET)
	rm -f callgrind.*.* $(PROFILE_TARGET)
	rm -rf $(PROFILE_BUILD)

.PHONY: profile run-profile-server run-profile-meteo run-profile-spotpris analyze-profile clean-profile