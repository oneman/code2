runcode:
	@clang -Wall `pkg-config --libs --cflags cairo gmp libpipewire-0.3` -lm source.c -o program.exe
