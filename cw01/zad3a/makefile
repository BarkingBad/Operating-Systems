static:
	gcc -Wall -c zadanie1.c -o zadanie1.o
	ar rcs zadanie1.a zadanie1.o
	rm -f *.o

shared:
	gcc -Wall -fPIC -c zadanie1.c
	gcc -Wall -shared -fPIC -o zadanie1.so zadanie1.o
	rm -f *.o

clear:
	rm -f *.o *.a *.so main

main_static:
	make static
	gcc -Wall -c zadanie2.c
	gcc -Wall zadanie2.o zadanie1.a -o zadanie2
	make tests

main_shared:
	make shared
	sudo cp zadanie1.so /usr/lib
	sudo ldconfig
	gcc -Wall -o zadanie2 zadanie2.c -L. zadanie1.so -I .
	make tests

main_dynamic:
	make shared
	gcc -Wall -o zadanie2  zadanie2d.c -I . -ldl
	make tests

tests:
	./zadanie2 create_block 10 search_directory / \* remove_block_at_index 0
	./zadanie2 create_block 1000 create_block 1000 create_block 1000 create_block 1000
	./zadanie2 create_block 10 search_directory \~ \* 
	./zadanie2 create_block 10 search_directory . \*

