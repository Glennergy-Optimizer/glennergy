# Compiler
CC = gcc
CFLAGS = -pg -g -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200112L

# Körbar fil
TARGET = Glennergy-Algoritm

# Default target
all:
	$(CC) $(CFLAGS) main.c average.c ../Libs/Sockets.c ../Libs/SHM.c ../Server/Log/Logger.c ../Server/SignalHandler.c -o $(TARGET) -lcurl -ljansson


install uninstall:
	@echo "Component installation is managed by the top-level Makefile." >&2
	@exit 2

# Rensa körbar fil
clean:
	rm -f $(TARGET)
