libscflags=`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`

compile:
	@gcc -g -Wall -O2 $(libscflags) -lm source.c -o program
	@sudo chown root ./program
	@sudo chmod +s ./program
	@clang -g -Wall -O2 $(libscflags) -lm source.c -o programc
	@sudo chown root ./programc
	@sudo chmod +s ./programc
	@sudo cp -a ./program /bin/program
	@sudo cp -a ./programc /bin/programc
	@sudo mkdir -p /map
	@sudo cp -a ./map/* /map/
	@echo "ok"
