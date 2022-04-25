#include "../libs/gestor_pedidos.h"
#include "../libs/funcoes.h"
#include "../libs/pedido.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> //open

struct gestor_pedidos {

    char transformacoes[10][20]; //Assume-se que cada transformação tem 20 caracteres no maximo
    int maximo[10]; //array com o maximo de ocorrencias de cada transformação
    int atual[10]; //array com a utilizaçao atual de cada transformação
    int ntransformacoes;
    
    int nPedidosMax; //numero de pedidos maximo
    int nPedidosEmExecucao; //numero de pedidos em execucao
    int nPedidosEmEspera; //numero de pedidos em espera
    PEDIDO *pedidosEmExecucao;
    PEDIDO *pedidosEmEspera;
};

/*
Cria o gestor de pedidos
*/
GESTOR_PEDIDOS createGestorPedidos(int pedidosMax, char* config_file) {
    //Cria o gestor estando os dois arrays vazios, mas com espaço para o numero de pedidos maximo
    GESTOR_PEDIDOS gestor = (GESTOR_PEDIDOS) malloc(sizeof(struct gestor_pedidos)
     + pedidosMax * sizeof(PEDIDO) //alocar espaço para o array pedidosEmExecucao
     + pedidosMax * sizeof(PEDIDO)); //alocar espaço para o array pedidosEmEspera

    int fd = open(config_file, O_RDONLY, 0666), i=0;
    char buffer[50];

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

        strcpy(gestor->transformacoes[i], transf); //copia a operação para a matriz de operações
        gestor->maximo[i] = max;
        gestor->atual[i] = 0;
        i++;
    }
    gestor->ntransformacoes = i;
    close(fd);

    gestor->pedidosEmExecucao = (PEDIDO*)(((char *) gestor) + sizeof(struct gestor_pedidos));
    gestor->pedidosEmEspera = (PEDIDO*)(((char *) gestor) + sizeof(struct gestor_pedidos)
    + pedidosMax * sizeof(PEDIDO));

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
    int i, ntransf = gp->ntransformacoes;
    int usados[ntransf]; //array de ocorrencias das transformaçoes do pedido (a preencher)
    int *maxT = gp->maximo; //array com o maximo de cada transformação
    int *atualT = gp->atual; //array com a atual utilização de cada transformação

    for (i = 0; i<ntransf; i++) {
        /*
        Para cada transformação do gestor de pedidos, vamos contar as ocorrencias que cada uma tem no pedido,
        e ao mesmo tempo verificamos se o pedido pode entrar, tendo em conta os recursos que utiliza.
        */
        char* transf = gp->transformacoes[i];
        int ocorrencias = ocorrenciasTransformacao(p, transf);
        usados[i] = ocorrencias;
        if (atualT[i] + ocorrencias > maxT[i]) { //nao pode inserir porque um recurso excede
            printf("[GESTOR_PEDIDOS]: Nao consegue inserir pedido, tem que ir pra fila de espera\n");
            return; //colocar na fila de espera, A FAZER....  break; e depois adicionar a fila de espera com certa ordem
        }
    }
    //A este ponto assume-se que o pedido pode ser realizado diretamente
    for (i=0; i<ntransf; i++) {
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

}