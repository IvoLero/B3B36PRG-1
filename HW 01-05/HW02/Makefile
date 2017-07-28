CFLAGS+= -pedantic -Wall -Werror -std=c99 -O3 
LIBS+= -lm -g

HW=hw02

main: main.c 
	${CC} main.c -o main ${CFLAGS} ${LIBS}
	
zip:
	zip ${HW}-brute.zip main.c
	
clean:
	rm -f *.o main
	rm -f ${HW}-brute.zip

.PHONY: clean zip
