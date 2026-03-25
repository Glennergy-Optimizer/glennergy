# Compiler
CC = g++
CFLAGS = -Wall -Wextra -std=c++17 

TARGET = Glennergy-Client


# Default target
all:
	$(CC) $(CFLAGS) main.cpp Request.cpp Response.cpp HTTP.cpp -o $(TARGET) -lcurl -ljansson
