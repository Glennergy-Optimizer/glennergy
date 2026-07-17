# C++ Meteo build
CXX      = g++
CC       = gcc
CXXFLAGS = -Wall -Wextra -std=c++20 -g
CFLAGS   = -Wall -Wextra -std=c11 -g -D_POSIX_C_SOURCE=200112L
DEPFLAGS = -MMD -MP

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
DEPS     = $(CXX_OBJS:.o=.d) $(C_OBJS:.o=.d)

TARGET   = Glennergy-Meteo

all: $(TARGET)

$(TARGET): $(CXX_OBJS) $(C_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ -lcurl

# C++ compilation
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

install uninstall:
	@echo "Component installation is managed by the top-level Makefile." >&2
	@exit 2

clean:
	rm -f $(TARGET) *.o *.d
