CFLAGS+= -pedantic -Wall -Werror -std=c99

HW=hw03

main: main.c 
	${CC} main.c -o main ${CFLAGS}
	
zip:
	zip ${HW}-brute.zip main.c
	
clean:
	rm -f *.o main
	rm -f ${HW}-brute.zip

.PHONY: clean zip
