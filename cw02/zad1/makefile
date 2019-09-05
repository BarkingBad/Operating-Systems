e = echo
t = time -p ./program
s = dane
d = dest
o = >> out 2>&1

main:
	gcc -Wall zadanie1.c -o program
	make tests

tests:
	make test r=1000 b=1
	make test r=10000 b=1
	make test r=2500 b=4
	make test r=25000 b=4
	make test r=10000 b=512
	make test r=80000 b=512
	make test r=10000 b=1024
	make test r=40000 b=1024
	make test r=1000 b=4096
	make test r=5000 b=4096
	make test r=1000 b=8192
	make test r=5000 b=8192
	
test:
	rm -f $(s) $(d)
	$(e) "Generuje $(r) lini o $(b) bajtach" $(o)
	$(t) generate $(s) $(r) $(b) $(o)

	$(e) "Kopiuje plik $(s) do $(d) o $(r) liniach i $(b) bajtach sys" $(o)
	$(t) copy $(s) $(d) $(r) $(b) sys $(o)

	$(e) "Sortuje plik $(d) o $(r) liniach i $(b) bajtach sys" $(o)
	$(t) sort $(d) $(r) $(b) sys $(o)

	$(e) "Kopiuje plik $(s) do $(d) o $(r) liniach i $(b) bajtach lib" $(o)
	$(t) copy $(s) $(d) $(r) $(b) lib $(o)

	$(e) "Sortuje plik $(d) o $(r) liniach i $(b) bajtach lib" $(o)
	$(t) sort $(d) $(r) $(b) lib $(o)

