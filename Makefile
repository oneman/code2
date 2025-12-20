libscflags=`pkg-config --libs --cflags cairo gmp xcb-image`

compile:
	@gcc -g -Wall -O2 $(libscflags) -lm source.c -o program
	@sudo chown root ./program
	@sudo chmod +s ./program
	@echo "ok"
