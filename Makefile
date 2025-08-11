TARGET = bin/shoter_с

BINDIR = bin

SRCS = src/*.c src/lib/*.c

INCLUDES = -I./include/

# unix
LIBS = -L./lib/ -Wl,-rpath=./lib/ -lhdf5 -lhdf5_hl -lpthread 

CC = gcc
CFLAGS = -g -Wall -O3 -march=native -ffast-math 

build: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LIBS) $(SRCS) -o $(TARGET)

run: $(TARGET)
	$(TARGET)
