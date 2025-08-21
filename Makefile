compile:
	@gcc -g -Wall -I/demo\
		`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`\
		 -lm source.c -o program
	@sudo chown root ./program
	@sudo chmod +s ./program
	@clang -g -Wall -I/demo\
		`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`\
		 -lm source.c -o programc
	@sudo chown root ./programc
	@sudo chmod +s ./programc
	@echo "ok"
