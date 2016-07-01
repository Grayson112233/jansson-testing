main: main.o
	gcc -Ljannson -Wall main.o -o main

main.o:
	gcc -Wall -c main.c
