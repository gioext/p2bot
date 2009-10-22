p2bot: p2bot.c
	gcc -Wall -m32 -O2 -I/opt/local/include -L/opt/local/lib -llua -o p2bot p2bot.c

util.so: util.c
	 gcc -Wall -m32 -O2 -shared -fPIC -I/opt/local/include -L/opt/local/lib -llua -o util.so util.c -llua
