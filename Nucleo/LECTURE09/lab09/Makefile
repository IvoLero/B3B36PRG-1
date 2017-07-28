#CC:=ccache $(CC)
CC:=gcc 
CFLAGS+=-O2
CFLAGS+=-std=gnu99
#CFLAGS+=-pedantic -Wall
LDFLAGS+=-lpthread

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

TARGET=main

bin: $(TARGET)

$(OBJS): %.o: %.c
	$(CC) -c $< $(CFLAGS) $(CPPFLAGS) -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	$(RM) $(OBJS) $(TARGET)

