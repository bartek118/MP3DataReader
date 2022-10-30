CC = gcc
CFLAGS = -Wall -Wpedantic -std=c99
LDFLAGS = 
SOURCES = main.c
OBJ = $(SOURCES:.c=.o)
EXECUTABLE = mp3

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJ) 
	$(CC) $(LDFLAGS) $(OBJ) -o $@

.cpp.o:  
	$(CC) $(CFLAGS) $< -c $@ 

clean: 
	rm -f *.o $(EXECUTABLE)
