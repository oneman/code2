compile:
	@clang -Wall `pkg-config --libs --cflags cairo gmp xcb wayland-client libpipewire-0.3` -lm source.c -o programa
	@gcc -Wall `pkg-config --libs --cflags cairo gmp xcb wayland-client libpipewire-0.3` -lm source.c -o programb
