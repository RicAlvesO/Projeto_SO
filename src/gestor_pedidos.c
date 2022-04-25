#include "../libs/gestor_pedidos.h"
#include "../libs/funcoes.h"
#include "../libs/pedido.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> //open


struct gestor_pedidos {
    
    int maximo[7]; //array com o maximo de ocorrencias de cada transformação
    int atual[7]; //array com a utilizaçao atual de cada transformação
    
    int nPedidosMax; //numero de pedidos maximo
    int nPedidosEmExecucao; //numero de pedidos em execucao
    int nPedidosEmEspera; //numero de pedidos em espera
    PEDIDO *pedidosEmExecucao;
    PEDIDO *pedidosEmEspera;
};

/*
Cria o gestor de pedidos
*/
GESTOR_PEDIDOS createGestorPedidos( char* config_file) {
    //Cria o gestor estando os dois arrays vazios, mas com espaço para o numero de pedidos maximo
    GESTOR_PEDIDOS gestor = (GESTOR_PEDIDOS) malloc(sizeof(struct gestor_pedidos)); 
    int fd = open(config_file, O_RDONLY, 0666),i=0;
    char buffer[50];
    int pedidosMax=0;

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
        i=transf_to_code(transf);
        // Caso apareca transformação desconhecida
        if(i==-1){
            write(2,"ERROR: UNKNOWN TRANSFORMATION IN CONFIGURATION FILE!!!",55);
        }
        gestor->maximo[i] = max;
        pedidosMax+=max;
        gestor->atual[i] = 0;
    }
    close(fd);

    gestor->pedidosEmExecucao = malloc(sizeof(PEDIDO)*pedidosMax);
    gestor->pedidosEmEspera = malloc(sizeof(PEDIDO));

    gestor->nPedidosMax = pedidosMax;
    gestor->nPedidosEmEspera = 0;
    gestor->nPedidosEmExecucao = 0;
    return gestor;
}

/*
Tenta inserir um pedido no gestor de pedidos. Apenas insere se for possivel inserir, tendo em conta o maximo
de cada transformação
*/
void inserirPedido(GESTOR_PEDIDOS gp, PEDIDO p) {
    int i;
    int usados[7]; //array de ocorrencias das transformaçoes do pedido (a preencher)
    int *maxT = gp->maximo; //array com o maximo de cada transformação
    int *atualT = gp->atual; //array com a atual utilização de cada transformação

    for (i = 0; i<7; i++) {
        int ocorrencias = ocorrenciasTransformacao(p, i);
        usados[i] = ocorrencias;
        if (atualT[i] + ocorrencias > maxT[i]) { //nao pode inserir porque um recurso excede
            write(1,"[GESTOR_PEDIDOS]: Nao consegue inserir pedido, tem que ir pra fila de espera\n",78);
            return; //colocar na fila de espera, A FAZER....  break; e depois adicionar a fila de espera com certa ordem
        }
    }
    //A este ponto assume-se que o pedido pode ser realizado diretamente
    for (i=0; i<7; i++) {
        atualT[i] += usados[i]; //adiciona o pedido, muda o array de utilização das transformaçoes
    }
    
    gp->pedidosEmExecucao[gp->nPedidosEmExecucao++] = p; //Esta parte pode ser mudada. É suposto adicionar o pedido ao gestor.
    executarPedido(p);

}

/*
Esta função recebe o gestor de pedidos, e o pid do processo que realizou o pedido
Depois percorre todos os pedidos a serem executados no gestor de pedidos e encontra o que tem o pid dado como argumento,
e elimina-o do gestor.
*/
void removerPedido(GESTOR_PEDIDOS gp, int pidPedido) {
    int n = gp->nPedidosEmExecucao;
    PEDIDO* pedidosEmExecucao = gp->pedidosEmExecucao;

    //remover o pedido cujo pid corresponde a pidPedido.
    //freePedido(p)
}

void freeGestor(GESTOR_PEDIDOS g){
    free(g->pedidosEmEspera);
    free(g->pedidosEmExecucao);
    free(g);
}

int* getMaximo(GESTOR_PEDIDOS gp) {
    return gp->maximo;
}

int* getAtual(GESTOR_PEDIDOS gp) {
    return gp->atual;
}

int getNPedidosEmExecucao(GESTOR_PEDIDOS gp) {
    return gp->nPedidosEmExecucao;
}

PEDIDO* getPedidosEmExecucao(GESTOR_PEDIDOS gp) {
    return gp->pedidosEmExecucao;
}