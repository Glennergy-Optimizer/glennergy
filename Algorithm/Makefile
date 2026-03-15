CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200809L

PREFIX ?= /usr/local/bin
TARGET = Glennergy-Algorithm

all:
	$(CC) $(CFLAGS) main.c testreader.c average.c solar.c optimizer.c utils.c ../Libs/Sockets.c ../Server/Log/Logger.c ../Server/SignalHandler.c -o $(TARGET) -ljansson -lpthread -lm

install:
	install -d $(DESTDIR)$(PREFIX)
	install -m 755 $(TARGET) $(DESTDIR)$(PREFIX)

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean