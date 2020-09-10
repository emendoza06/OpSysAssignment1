CC = gcc 
CFLAGS = -g
TARGET = mydu
OBJS = mydu.o

$(TARGET): $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)

mydu.o : mydu.c
	$(CC) $(CFLAGS) -c mydu.c

clean: 
	/bin/rm -f *.o $(TARGET)
