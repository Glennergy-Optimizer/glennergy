# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L

# Källkoder
CACHE_DIR = .
API_DIR = ../API

# Körbar fil
PREFIX ?= /usr/local/bin
TARGET = Glennergy-InputCache
SRC = main.c

INSTALLDIR = $(PREFIX)
CACHEDIR = /var/cache/glennergy

METEOCACHEDIR = $(CACHEDIR)/meteo
SPOTPRISCACHEDIR = $(CACHEDIR)/spotpris

# Default target
all:
	$(CC) $(CFLAGS) $(CACHE_DIR)/InputCache.c $(CACHE_DIR)/main.c $(API_DIR)/Meteo/Meteo.c $(API_DIR)/Spotpris/Spotpris.c ../Libs/Fetcher.c ../Libs/Pipes.c ../Libs/Shm.c ../Libs/Sockets.c ../Libs/Homesystem.c ../Server/Log/Logger.c ../Server/SignalHandler.c -o $(TARGET) -lcurl -ljansson

install:
	# Create directory
	install -d $(DESTDIR)$(INSTALLDIR)

	# Create cache directory
	install -d $(DESTDIR)$(CACHEDIR)
	install -d $(DESTDIR)$(METEOCACHEDIR)
	install -d $(DESTDIR)$(SPOTPRISCACHEDIR)

	# Install binary
	install -m 755 $(TARGET) $(DESTDIR)$(INSTALLDIR)


uninstall:
	rm -rf $(DESTDIR)$(INSTALLDIR)/$(TARGET)

# Rensa körbar fil
clean:
	rm -f $(TARGET)
