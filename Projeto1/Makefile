CFLAGS= -Wall -std=gnu99
CircuitRouter-SimpleShell: CircuitRouter-SimpleShell.o lib/commandlinereader.o linkedlist.o
	gcc lib/commandlinereader.o CircuitRouter-SimpleShell.o linkedlist.o -lm -o CircuitRouter-SimpleShell
lib/commandlinereader.o: lib/commandlinereader.c lib/commandlinereader.h
	gcc $(CFLAGS) -c lib/commandlinereader.c -o lib/commandlinereader.o
CircuitRouter-SimpleShell.o: CircuitRouter-SimpleShell.c CircuitRouter-SimpleShell.h
	gcc $(CFLAGS) -c CircuitRouter-SimpleShell.c -o CircuitRouter-SimpleShell.o
linkedlist.o: linkedlist.c linkedlist.h
	gcc $(CFLAGS) -c linkedlist.c -o linkedlist.o

