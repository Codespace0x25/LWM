


all: build build/LWM


build/LWM: ./src/main.c
	gcc -o build/LWM  ./src/main.c -lX11 -llua5.3



build: 
	mkdir -p build
