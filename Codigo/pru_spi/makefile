TARGET  = medcape
CC      = gcc
CFLAGS  = -g -Wall 
LDFLAGS = -lpthread -lprussdrv
OBJDIR  = obj
SOURCES = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJS    = $(addprefix $(OBJDIR)/,$(notdir $(SOURCES:.c=.o)))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJS): $(HEADERS) makefile

clean: 
	-rm -f ./obj/*.o $(TARGET)



