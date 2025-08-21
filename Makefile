libscflags=`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`
install:
	@gcc -g -Wall $(libscflags) -lm source.c -o program
	@sudo chown root ./program
	@sudo chmod +s ./program
	@sudo cp -a ./program /bin/program
	@clang -g -Wall $(libscflags) -lm source.c -o programc
	@sudo chown root ./programc
	@sudo chmod +s ./programc
	@echo "ok"
