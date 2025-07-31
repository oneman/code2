compile:
	@echo "gcc ok"
	@gcc -Wall -I/demo \
		`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3` -lm source.c -o programb
	@echo "ok clang"
	@clang -Wall -I/demo \
		`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3` -lm source.c -o programa
	@echo "ok clang extra"
	@clang -Wall -I/demo hexde.c -o hexdec
