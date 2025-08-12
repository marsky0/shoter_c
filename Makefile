TARGET = bin/shoter

BINDIR = bin

SRCS = src/*.c src/lib/*.c

INCLUDES = -I./include/

# unix
LIBS = -L./lib/ -lhdf5 -lhdf5_hl -lpthread #-Wl,-rpath=./lib/

CC = gcc
CFLAGS = -g -Wall -O3 -march=native -ffast-math 

build: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	$(TARGET)

clear:
	rm -f $(TARGET) *.o
