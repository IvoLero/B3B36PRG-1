#CFLAGS+= -Wall -Werror -std=c99 -g
CFLAGS+= -Wall -Werror -g

OBJS=$(patsubst %.c,%.o,$(wildcard *.c))

PRG_SERIAL=prg_serial.o

BINS=lab08-main
bin: ${BINS}

lab08-main: lab08-main.o prg_serial.o
	${CC} ${LDFLAGS} $< ${PRG_SERIAL} -o $@

${OBJS}: %.o: %.c
	${CC} ${CFLAGS} ${CPPFLAGS} -c $< -o $@

clean:
	rm -f *.o $(OBJS) ${BINS} 
