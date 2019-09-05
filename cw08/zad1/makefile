all:
	gcc zad1.c -o zad1 -lpthread -lm

tests:
	echo "Obraz wejściowy miał 640x480 pikseli.\nKażda nowa seria zaczynająca wiersz od 0 to naprzemian wersja block lub interleaved. Jeżeli seria kończy się na indeksie n to znaczy że była uruchomiona z n+1 wątków." > raport.txt
	echo "\nFiltr 3x3" >> raport.txt
	./zad1 1 block input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 1 interleaved input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 2 block input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 2 interleaved input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 4 block input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 4 interleaved input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 8 block input3.pgm output.pgm filter.txt >> raport.txt
	./zad1 8 interleaved input3.pgm output.pgm filter.txt >> raport.txt
	echo "\nFiltr 15x15" >> raport.txt
	./zad1 1 block input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 1 interleaved input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 2 block input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 2 interleaved input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 4 block input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 4 interleaved input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 8 block input3.pgm output.pgm large_filter.txt >> raport.txt
	./zad1 8 interleaved input3.pgm output.pgm large_filter.txt >> raport.txt
	echo "\nFiltr 65x65" >> raport.txt
	./zad1 1 block input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 1 interleaved input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 2 block input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 2 interleaved input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 4 block input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 4 interleaved input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 8 block input3.pgm output.pgm huge_filter.txt >> raport.txt
	./zad1 8 interleaved input3.pgm output.pgm huge_filter.txt >> raport.txt