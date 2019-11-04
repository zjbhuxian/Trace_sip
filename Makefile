CC=gcc
CFLAGS=-Wall -O -g
DFLAGS=-L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -lz -lm -lrt -latomic -ldl
SRCS=$(wildcard *.c)
OBJS=$(patsubst %.c, %.o, $(SRCS))
TARGET=test

all:$(TARGET)
	
$(TARGET):$(OBJS)
	$(CC) $^ $(DFLAGS) -o $@

$(OBJS):$(SRCS)
	$(CC) $^ $(CFLAGS) -c

clean:
	rm -rf *.o

clean_all:clean
	rm -rf $(TARGET)
