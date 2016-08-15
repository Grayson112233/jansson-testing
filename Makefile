main: main.o
	gcc -ljansson -lcurl -Wall main.o -o main -std=c99

main.o: main.c
	gcc -Wall -c main.c -std=c99
