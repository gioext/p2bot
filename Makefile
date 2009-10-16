CFLAG = -g -Wall -O2 -m32
INCLUDE = -I/opt/local/include
LIB = -L/opt/local/lib -llua

p2bot: p2bot.c util.o
	gcc ${CFLAG} ${INCLUDE} ${LIB} -o $@ p2bot.c util.o

util.o: util.c util.h
	gcc -c ${CFLAG} util.c
