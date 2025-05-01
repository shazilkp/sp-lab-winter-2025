CC = gcc
TARGET = program

all: $(TARGET)

#build the taget
$(TARGET): main.o utils.o math.o
	$(CC) -o $(TARGET) main.o utils.o math.o

#Compiling object files
main.o: main.c utils.h math.h
	$(CC) -c main.c
	
utils.o: utils.c utils.h math.h
	$(CC) -c utils.c

math.o: math.c math.h
	$(CC) -c math.c

clean:
	rm -f main.o utils.o math.o $(TARGET)
