optimal:
	@clang -O3 -g -I/demo -Wall \
		`pkg-config --libs --cflags cairo gmp xcb wayland-client libpipewire-0.3` -lm source.c -o programa
	@gcc -O3 -g -Wall -I/demo\
		`pkg-config --libs --cflags cairo gmp xcb wayland-client libpipewire-0.3` -lm source.c -o programb
compile:
	@clang -Wall -I/demo \
		`pkg-config --libs --cflags cairo gmp xcb wayland-client libpipewire-0.3` -lm source.c -o programa
	@gcc -Wall -I/demo \
		`pkg-config --libs --cflags cairo gmp xcb wayland-client libpipewire-0.3` -lm source.c -o programb
