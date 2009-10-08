CFLAG = -g -Wall -O2 -m32
p2bot: p2bot.c
	gcc ${CFLAG} -o $@ $<
