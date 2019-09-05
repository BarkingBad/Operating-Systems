static:
	gcc -Wall -c zadanie1.c -o zadanie1.o
	ar rcs zadanie1.a zadanie1.o
	rm -f *.o

shared:
	gcc -Wall -c zadanie1.c -o zadanie1.o
	gcc -Wall -shared -fPIC -o zadanie1.so zadanie1.o
	rm -f *.o

clear:
	rm -f *.o *.a *.so
