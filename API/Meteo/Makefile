# Compiler
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L

# Källkoder
CACHE_DIR = .
API_DIR = ../API

# Körbar fil
PREFIX ?= /usr/local/bin
TARGET = Glennergy-Meteo

INSTALLDIR = $(PREFIX)
CONFIGDIR = /etc

# Default target
all:
	$(CC) $(CFLAGS) main.c Meteo.c ../../Libs/Pipes.c ../../Libs/Fetcher.c ../../Server/Log/Logger.c -o $(TARGET) -lcurl -ljansson


install:
	# Create directory
	install -d $(DESTDIR)$(INSTALLDIR)

	install -m 644 Glennergy-Fastigheter.json $(DESTDIR)$(CONFIGDIR)

	# Install binary
	install -m 755 $(TARGET) $(DESTDIR)$(INSTALLDIR)


uninstall:
	rm -rf $(DESTDIR)$(INSTALLDIR)/$(TARGET)
	rm -f $(DESTDIR)$(CONFIGDIR)/Glennergy-Fastigheter.json

# Rensa körbar fil
clean:
	rm -f $(TARGET)