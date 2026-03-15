# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L

# Källkoder
CACHE_DIR = .
API_DIR = ../API

# Körbar fil
PREFIX ?= /usr/local/bin
TARGET = Glennergy-Spotpris

INSTALLDIR = $(PREFIX)

# Default target
all:
	$(CC) $(CFLAGS) Spotpris.c spotpristest.c ../../Libs/Pipes.c ../../Libs/Fetcher.c ../../Server/Log/Logger.c -o $(TARGET) -lcurl -ljansson


install:
	# Create directory
	install -d $(DESTDIR)$(INSTALLDIR)

	# Install binary
	install -m 755 $(TARGET) $(DESTDIR)$(INSTALLDIR)


uninstall:
	rm -rf $(DESTDIR)$(INSTALLDIR)/$(TARGET)

# Rensa körbar fil
clean:
	rm -f $(TARGET)