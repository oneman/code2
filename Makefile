compilecode:
	@clang -Wall `pkg-config --libs --cflags cairo gmp xcb libpipewire-0.3` -lm source.c -o programa
compilecodeagain:
	@gcc -Wall `pkg-config --libs --cflags cairo gmp xcb libpipewire-0.3` -lm source.c -o programb

