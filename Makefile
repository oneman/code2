compile:
	@echo "cc ok"
	@cc -Wall -I/demo\
		`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`\
		 -lm source.c -o program
	@echo "ok clang"
	@clang -Wall -I/demo\
		`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`\
		 -lm source.c -o programc
	@echo "ok clang extra"
	@clang -Wall -I/demo\
			`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`\
			 -lm hexde.c -o hexdec
	@clang -Wall -I/demo\
			`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`\
			 -lm randotxt.c -o randotxt
