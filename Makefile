clack: Again
	@gcc -Wall `pkg-config --libs --cflags libpipewire-0.3` fp.c -o fp.exe
	@clang -Wall rad.c -o rad.exe
Again: Great
	@echo 'Again'
Great: America
	@echo 'Great'
America:
	@echo 'America'
