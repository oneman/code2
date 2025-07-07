runcode:
	@clang -Wall `pkg-config --libs --cflags cairo gmp libpipewire-0.3 wayland-client` -lm source.c -o program.exe
