#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h> //O_RDONLY, ...
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../libs/servidor.h"
#include "../libs/funcoes.h"
#include "../libs/pedido.h"
#include "../libs/gestor_pedidos.h"

struct server {
    //Assume-se que há 7 operações
    int maximo[7]; //array com o maximo de ocorrencias de cada transformação
    int atual[7]; //array com a utilizaçao atual de cada transformação
    int tamanhoPedidosStr;
    char pedidos[];
};

SERVER createServerFromGestor(GESTOR_PEDIDOS gp) {
    int i;
    int ntransf = 7;

    int* maxArray = getMaximo(gp);
    int* atualArray = getAtual(gp);

    int tamanhoPedidosStr;
    char* pedidosStr = getAllPedidosStr(gp, &tamanhoPedidosStr);

    SERVER server = malloc(sizeof(struct server) + sizeof(char) * tamanhoPedidosStr);

    createAtualArray(gp);

    for (i=0; i<ntransf; i++) {
        server->maximo[i] = maxArray[i];
        server->atual[i] = atualArray[i];
    }
    server->tamanhoPedidosStr = tamanhoPedidosStr;
    strcpy(server->pedidos, pedidosStr);
    return server;
}

/*
Recebe o path para o ficheiro de configuração e cria o server
*/
SERVER createServer(char* config_file) {
    int fd = open(config_file, O_RDONLY, 0666);
    char buffer[50];
    SERVER server = malloc(sizeof(struct server));

    while (readln(fd, buffer, 50) > 0) { //enquanto nao chega a EOF...
        /* Assume-se que o ficheiro está bem parsado
        Exemplo de ficheiro bem parsado:
        ---------------
        nop 3
        bcompress 4
        bdecompress 4
        ---------------
        */
        char* transf = strtok(buffer, " "); //strtok pode dar problemas com varios processos?
        int max = atoi(strtok(NULL, " "));

        int transf_code = transf_to_code(transf);
        server->maximo[transf_code] = max;
        server->atual[transf_code] = 0;
    }

    close(fd);

    return server;
}

/*
$ ./sdstore status

Printa o estado do server
*/
void printServerStatus(SERVER server) {
    //printf("A printar o Server\n");
    int i=0, max = 7;
    printf("%s", server->pedidos);
    for (;i<max;i++) {
        int maximo = server->maximo[i];
        int atual = server->atual[i];
        printf("transf %s: %d/%d (running/max)\n",code_to_transf(i), atual, maximo);
    }
}

/*
Recebe uma struct server do fifo
*/
SERVER readServer(int fd) {
    //o ficheiro começa fechado, é preciso abrir
    //ja sabemos o path do fifo
    
    //if(mkfifo("pipe.fifo", 0666) != 0 && errno != EEXIST) {
    //    write(2,"nao conseguiu criar FIFO 'pipe.fifo': ",39);
    //}

    //int fd = open("pipe.fifo",O_RDONLY);
    //if (fd < 0) {
    //    write(2, "nao conseguiu abrir FIFO 'pipe.fifo' para ler: ", 48);
    //}

    SERVER server1 = malloc(sizeof(struct server));
    int ret;
    if ((ret = read(fd, server1, sizeof(struct server))) != sizeof(struct server)) {
        write(2, "nao conseguiu ler o server do fifo: ", 36);
    }
    SERVER server2 = malloc(sizeof(struct server) + server1->tamanhoPedidosStr);
    *server2 = *server1;
    free(server1);
    if (read(fd, server2->pedidos, server2->tamanhoPedidosStr) != server2->tamanhoPedidosStr) {
        write(2, "nao conseguiu ler pedidos do fifo: ", 35);
    }

    //é da responsabilidade do utilizador fechar o fd

    return server2;
}

/*
Escreve uma struct server para o fifo
*/
void writeServer(SERVER server, int fd) {

    //if(mkfifo("pipe.fifo", 0666) != 0 && errno != EEXIST) {
    //    write(2,"nao conseguiu criar FIFO 'pipe.fifo': ",39);
    //}

    //int fd = open("pipe.fifo",O_WRONLY);
    //if (fd < 0) {
    //    write(2, "nao conseguiu abrir FIFO 'pipe.fifo' para escrever: ", 53);
    //}

    if (write(fd, server, sizeof(struct server) + server->tamanhoPedidosStr) != (sizeof(struct server) + server->tamanhoPedidosStr)) {
        write(2, "nao conseguiu escrever o servidor no fifo\n", 42);
    }

    //cabe ao utilizador dar free a este server, e fechar o fd.
}

void freeServer(SERVER* server) {
    free(*server); //os arrays dentro da struct desaparecem quando se sai do scope?
    *server = NULL;
}