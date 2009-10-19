CFLAG = -g -Wall -O2 -m32
INCLUDE = -I/opt/local/include
LIB = -L/opt/local/lib

p2bot: p2bot.c util.o gstack.o
	gcc ${CFLAG} ${INCLUDE} ${LIB} -o $@ p2bot.c util.o gstack.o

util.o: util.c util.h
	gcc -c ${CFLAG} util.c

gstack.o: gstack.c gstack.h
	gcc -c ${CFLAG} gstack.c
