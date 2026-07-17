# Compiler
CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200112L

# Källkoder
CACHE_DIR = .
API_DIR = ../API

# Körbar fil
TARGET = Glennergy-InputCache
SRC = main.c

# Default target
all:
	$(CC) $(CFLAGS) $(CACHE_DIR)/InputCache.c $(CACHE_DIR)/main.c $(API_DIR)/Meteo/Meteo.c $(API_DIR)/Spotpris/Spotpris.c ../Libs/Fetcher.c ../Libs/Pipes.c ../Libs/Sockets.c ../Libs/Homesystem.c ../Server/Log/Logger.c ../Server/SignalHandler.c -o $(TARGET) -lcurl -ljansson

install uninstall:
	@echo "Component installation is managed by the top-level Makefile." >&2
	@exit 2

# Rensa körbar fil
clean:
	rm -f $(TARGET)
