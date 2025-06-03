runcode: 'Start'
	@clang -Wall `pkg-config --libs --cflags cairo gmp libpipewire-0.3` source.c -o program.exe
