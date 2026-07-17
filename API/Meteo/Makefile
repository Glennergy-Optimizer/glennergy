# Compiler
CC = gcc
CFLAGS = -pg -g -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200112L

# Källkoder
CACHE_DIR = .
API_DIR = ../API

# Körbar fil
TARGET = Glennergy-Meteo

# Default target
all:
	$(CC) $(CFLAGS) main.c Meteo.c ../../Libs/Pipes.c ../../Libs/Fetcher.c ../../Server/Log/Logger.c -o $(TARGET) -lcurl -ljansson


install uninstall:
	@echo "Legacy C Meteo is not part of the production installation." >&2
	@echo "Use the top-level Makefile to install the selected C++ implementation." >&2
	@exit 2

# Rensa körbar fil
clean:
	rm -f $(TARGET)
