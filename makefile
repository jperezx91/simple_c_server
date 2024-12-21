CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = simple_c_server

all: $(TARGET)

$(TARGET): simple_c_server.o hashmap.o
	$(CC) simple_c_server.o hashmap.o archivohttp.o server.o -o $(TARGET)


fast_server.o: simple_c_server.c hashmap.h
	$(CC) $(CFLAGS) -c simple_c_server.c

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c archivohttp.c server.c

clean:
	rm -f *.o $(TARGET)
	echo "Cleaned"