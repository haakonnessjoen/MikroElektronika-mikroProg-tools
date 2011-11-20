all: setvolt.c
	gcc -o setvolt setvolt.c -lusb-1.0
