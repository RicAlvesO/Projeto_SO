all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o obj/servidor.o obj/funcoes.o
	gcc -g -o bin/sdstored obj/sdstored.o obj/servidor.o obj/funcoes.o

obj/sdstored.o: src/sdstored.c
	gcc -Wall -g -c src/sdstored.c -o obj/sdstored.o

obj/servidor.o: src/servidor.c libs/servidor.h
	gcc -Wall -g -c src/servidor.c -o obj/servidor.o

obj/funcoes.o: src/funcoes.c libs/funcoes.h
	gcc -Wall -g -c src/funcoes.c -o obj/funcoes.o

bin/sdstore: obj/sdstore.o obj/servidor.o obj/funcoes.o
	gcc -g -o bin/sdstore obj/sdstore.o obj/servidor.o obj/funcoes.o

obj/sdstore.o: src/sdstore.c
	gcc -Wall -g -c src/sdstore.c -o obj/sdstore.o

clean:
	rm -f obj/*.o tmp/* bin/{sdstore,sdstored}