main: main.o
	gcc  -Wall main.o -o main -std=c99 -ljansson -lcurl
main.o: main.c
	gcc -Wall -c main.c -std=c99
