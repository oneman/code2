libscflags=`pkg-config --libs --cflags cairo gmp libdrm libpipewire-0.3`

compile:
	@gcc -g -Wall -O2 $(libscflags) -lm source.c -o program
	@sudo chown root ./program
	@sudo chmod +s ./program
	@clang -g -Wall -O2 $(libscflags) -lm source.c -o programc
	@sudo chown root ./programc
	@sudo chmod +s ./programc
	@echo "u can run make install (it cp's program to /bin and map to /)"
	@echo "r u can export OVERRIDE_PROGRAM_PIXMAP_BASEDIR=/where/ru/map"
	@echo "and run program from wher ever CWD may be, but it needs dat map"
	@echo "k thanks -dev"

install: compile
	@sudo chown root ./program
	@sudo chmod +s ./program
	@sudo cp -a ./program /bin/program
	@sudo mkdir -p /map
	@sudo cp -a ./map/* /map/
	@echo "ok"
