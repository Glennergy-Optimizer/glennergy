# Compiler
CC = gcc
CFLAGS = -pg -g -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200112L

# Körbar fil
PREFIX ?= /usr/local/bin
TARGET = Glennergy-Algoritm

INSTALLDIR = $(PREFIX)

# Default target
all:
	$(CC) $(CFLAGS) main.c average.c ../Libs/Sockets.c ../Libs/SHM.c ../Server/Log/Logger.c -o $(TARGET) -lcurl -ljansson


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