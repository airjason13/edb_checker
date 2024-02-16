edb_checker: edb_checker.o
	gcc edb_checker.o -lcjson -o edb_checker

edb_checker.o: edb_checker.c
	gcc -c edb_checker.c -lcjson -Wall -g -O3 -o edb_checker.o

clean:
	rm -f edb_checker.o edb_checker

