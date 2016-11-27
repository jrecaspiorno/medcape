TARGET  = medcape
CC      = gcc
CFLAGS  = -g -Wall -lpthread -lprussdrv
HEADERS = $(wildcard *.h)
SOURCES = $(wildcard *.c)
OBJS    = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS) 
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

.c.o: 
	$(CC) $(CFLAGS) -c  $< -o $@

$(OBJS): $(HEADERS) makefile

clean: 
	-rm -f *.o $(TARGET)
