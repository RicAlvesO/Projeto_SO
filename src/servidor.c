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

struct server {
    //Assume-se que há 10 operações no maximo
    char transformacoes[10][20]; //Assume-se que cada transformação tem 20 caracteres no maximo
    int maximo[10]; //array com o maximo de ocorrencias de cada transformação
    int atual[10]; //array com a utilizaçao atual de cada transformação
    int ntransformacoes;
};

/*
Recebe o path para o ficheiro de configuração e cria o server
*/
SERVER createServer(char* config_file) {
    int fd = open(config_file, O_RDONLY, 0666), i=0;
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

        strcpy(server->transformacoes[i], transf); //copia a operação para a matriz de operações
        server->maximo[i] = max;
        server->atual[i] = 0;
        i++;
    }
    server->ntransformacoes = i;

    close(fd);

    return server;
}

/*
$ ./sdstore status

Printa o estado do server
*/
void printServerStatus(SERVER server) {
    printf("A printar o Server\n");
    int i=0, max = server->ntransformacoes;
    for (;i<max;i++) {
        char* transf = server->transformacoes[i];
        int max = server->maximo[i];
        int atual = server->atual[i];
        printf("transf %s: %d/%d (running/max)\n",transf, atual, max);
    }
}

/*
Recebe uma struct server do fifo
*/
SERVER readServer() {
    //o ficheiro começa fechado, é preciso abrir
    //ja sabemos o path do fifo
    
    if(mkfifo("pipe.fifo", 0666) != 0 && errno != EEXIST) {
        write(2,"nao conseguiu criar FIFO 'pipe.fifo': ",39);
    }

    int fd = open("pipe.fifo",O_RDONLY);
    if (fd < 0) {
        write(2, "nao conseguiu abrir FIFO 'pipe.fifo' para ler: ", 48);
    }

    SERVER server = malloc(sizeof(struct server));
    if (read(fd, server, sizeof(struct server)) != sizeof(struct server)) {
        write(2, "nao conseguiu ler do fifo: ", 28);
    }

    close(fd);

    return server;
}

/*
Escreve uma struct server para o fifo
*/
void writeServer(SERVER server) {

    if(mkfifo("pipe.fifo", 0666) != 0 && errno != EEXIST) {
        write(2,"nao conseguiu criar FIFO 'pipe.fifo': ",39);
    }

    int fd = open("pipe.fifo",O_WRONLY);
    if (fd < 0) {
        write(2, "nao conseguiu abrir FIFO 'pipe.fifo' para escrever: ", 53);
    }

    if (write(fd, server, sizeof(struct server)) != sizeof(struct server)) {
        write(2, "nao conseguiu escrever no fifo: ", 33);
    }

    //cabe ao utilizador dar free a este server.
    close(fd);
}