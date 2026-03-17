# C++ Meteo build
CXX      = g++
CC       = gcc
CXXFLAGS = -Wall -Wextra -std=c++20 -g
CFLAGS   = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200112L

# Source directories (relative to this Makefile)
ROOT     = ../../..
LIBS_DIR = $(ROOT)/Libs
LOG_DIR  = $(ROOT)/Server/Log

# C++ sources
CXX_SRCS = main.cpp Meteo.cpp fetcher.cpp

# C sources (dependencies)
C_SRCS   = 

# Object files
CXX_OBJS = $(CXX_SRCS:.cpp=.o)
C_OBJS   = 

TARGET   = Glennergy-Meteo

PREFIX     ?= /usr/local/bin
INSTALLDIR  = $(PREFIX)
CONFIGDIR   = /etc

all: $(TARGET)

$(TARGET): $(CXX_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lcurl

# C++ compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

install:
	install -d $(DESTDIR)$(INSTALLDIR)
	install -m 644 ../Glennergy-Fastigheter.json $(DESTDIR)$(CONFIGDIR)
	install -m 755 $(TARGET) $(DESTDIR)$(INSTALLDIR)

uninstall:
	rm -rf $(DESTDIR)$(INSTALLDIR)/$(TARGET)
	rm -f $(DESTDIR)$(CONFIGDIR)/Glennergy-Fastigheter.json

clean:
	rm -f $(TARGET) *.o
