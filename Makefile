all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o obj/servidor.o obj/funcoes.o obj/pedido.o obj/gestor_pedidos.o obj/ll_pedidos.o
	gcc -g -o bin/sdstored obj/sdstored.o obj/servidor.o obj/funcoes.o obj/pedido.o obj/gestor_pedidos.o obj/ll_pedidos.o

obj/sdstored.o: src/sdstored.c
	gcc -Wall -g -c src/sdstored.c -o obj/sdstored.o

obj/servidor.o: src/servidor.c libs/servidor.h
	gcc -Wall -g -c src/servidor.c -o obj/servidor.o

obj/pedido.o: src/pedido.c libs/pedido.h
	gcc -Wall -g -c src/pedido.c -o obj/pedido.o

obj/ll_pedidos.o: src/ll_pedidos.c libs/ll_pedidos.h
	gcc -Wall -g -c src/ll_pedidos.c -o obj/ll_pedidos.o

obj/gestor_pedidos.o: src/gestor_pedidos.c libs/gestor_pedidos.h
	gcc -Wall -g -c src/gestor_pedidos.c -o obj/gestor_pedidos.o

obj/funcoes.o: src/funcoes.c libs/funcoes.h
	gcc -Wall -g -c src/funcoes.c -o obj/funcoes.o

bin/sdstore: obj/sdstore.o obj/servidor.o obj/funcoes.o obj/pedido.o obj/gestor_pedidos.o obj/ll_pedidos.o
	gcc -g -o bin/sdstore obj/sdstore.o obj/servidor.o obj/funcoes.o obj/pedido.o obj/gestor_pedidos.o obj/ll_pedidos.o

obj/sdstore.o: src/sdstore.c
	gcc -Wall -g -c src/sdstore.c -o obj/sdstore.o

clean:
	rm -f obj/*.o tmp/* bin/{sdstore,sdstored} fifos/*