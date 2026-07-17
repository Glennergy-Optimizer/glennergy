# Compiler
CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200112L

# Källkoder
CACHE_DIR = .
API_DIR = ../API

# Körbar fil
TARGET = Glennergy-Spotpris

# Default target
all:
	$(CC) $(CFLAGS) Spotpris.c spotpristest.c ../../Libs/Pipes.c ../../Libs/Fetcher.c ../../Server/Log/Logger.c -o $(TARGET) -lcurl -ljansson


install uninstall:
	@echo "Component installation is managed by the top-level Makefile." >&2
	@exit 2

# Rensa körbar fil
clean:
	rm -f $(TARGET)
