#include "../libs/gestor_pedidos.h"
#include "../libs/funcoes.h"
#include "../libs/pedido.h"
#include "../libs/ll_pedidos.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> //open


struct gestor_pedidos {
    
    int maximo[7]; //array com o maximo de ocorrencias de cada transformação
    int atual[7]; //array com a utilizaçao atual de cada transformação

    char transformation_folder[128];

    NODO pedidosExecucao;
    NODO pedidosEspera;
};

GESTOR_PEDIDOS createGestorPedidos( char* config_file, char* transf_folder) {
    //Cria o gestor estando os dois arrays vazios, mas com espaço para o numero de pedidos maximo
    GESTOR_PEDIDOS gestor = (GESTOR_PEDIDOS) malloc(sizeof(struct gestor_pedidos)); 
    int fd = open(config_file, O_RDONLY, 0666),i=0;
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
        i=transf_to_code(transf);
        // Caso apareca transformação desconhecida
        if(i==-1){
            write(2,"[ERROR]: UNKNOWN TRANSFORMATION IN CONFIGURATION FILE!!!",55);
        }
        gestor->maximo[i] = max;
        gestor->atual[i] = 0;
    }
    close(fd);

    //strncpy evita escrever em regiao de memoria nao alocada
    strncpy(gestor->transformation_folder,transf_folder, sizeof(gestor->transformation_folder));

    gestor->pedidosEspera = NULL;
    gestor->pedidosExecucao = NULL;

    return gestor;
}

/*
Tenta inserir na fila de execucao o pedido com maior prioridade que esta na fila de espera.
retorna 1 se conseguiu inserir o pedido, retorna 0 se nao conseguiu
*/
int tryInserirPedido(GESTOR_PEDIDOS gp) {
    int i, usados[7];
    int* atualT = gp->atual;
    int* maxT = gp->maximo;
    NODO nodo = gp->pedidosEspera;
    if (nodo == NULL) return 0; //A fila de espera esta vazia.
    PEDIDO pedido = getNodoPedido(nodo); //pedido com maior prioridade para entrar na fila de espera. (é o nodo que esta na cabeça da lista ligada)

    createAtualArray(gp); //atualiza o array atual com as transformacoes a ser usadas.

    for (i = 0; i<7; i++) {
        int ocorrencias = ocorrenciasTransformacao(pedido, i);
        usados[i] = ocorrencias;
        if (atualT[i] + usados[i] > maxT[i]) { //nao pode inserir porque um recurso excede
            //O pedido ainda nao pode ser introduzido na fila de execucao
            return 0; //colocar na fila de espera, A FAZER....  break; e depois adicionar a fila de espera com certa ordem
        }
    }
    //se chegou a este ponto, entao o pedido tem espaço para entrar na fila de execucao, entao introduz-se
    int pid = getPidPedido(pedido);
    removerNodo(&(gp->pedidosEspera), pid); //remover o pedido da fila de espera, dado que vai ser introduzido na fila de execucao
    inserirPedido(gp, pedido);
    return 1;
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
    NODO nodo = createNodo(p);
    for (i = 0; i<7; i++) {
        int ocorrencias = ocorrenciasTransformacao(p, i);
        usados[i] = ocorrencias;
        if (atualT[i] + ocorrencias > maxT[i]) { //nao pode inserir porque um recurso excede
            write(1,"[GESTOR_PEDIDOS]: Nao consegue inserir pedido, tem que ir pra fila de espera\n",78);
            alertPedidoEmEspera(p);
            inserirNodo(&(gp->pedidosEspera),nodo);
            return; //colocar na fila de espera, A FAZER....  break; e depois adicionar a fila de espera com certa ordem
        }
    }
    //A este ponto assume-se que o pedido pode ser realizado diretamente
    for (i=0; i<7; i++) {
        atualT[i] += usados[i]; //adiciona o pedido, muda o array de utilização das transformaçoes
    }
    inserirNodo(&(gp->pedidosExecucao),nodo);
    alertPedidoInserido(p);
    executarPedido(p, gp->transformation_folder);
}

/*
Devolve um boleano que representa se o servidor esta vazio, isto é, se nao ha pedidos na fila de espera 
nem na fila de execucao
*/
int gestorIsEmpty(GESTOR_PEDIDOS gp) {
    return gp->pedidosExecucao == NULL && gp->pedidosEspera == NULL;
}

/*
Esta função recebe o gestor de pedidos, e o pid do processo que realizou o pedido
Depois percorre todos os pedidos a serem executados no gestor de pedidos e encontra o que tem o pid dado como argumento,
e elimina-o do gestor.
*/
void removerPedido(GESTOR_PEDIDOS gp, int pidPedido) {
    NODO nodo = removerNodo(&(gp->pedidosExecucao),pidPedido);
    if (nodo == NULL) return;
    PEDIDO pedido = getNodoPedido(nodo);
    changeOcorrenciasTransformacoes(gp->atual, pedido, 0);
    while (tryInserirPedido(gp));

}

char* getAllPedidosStr(GESTOR_PEDIDOS gp, int *tamanhoString) {
    *tamanhoString = 0;
    NODO aux, tmp;
    aux = tmp = gp->pedidosExecucao;
    while (tmp != NULL) {
        PEDIDO pedidoAtual = getNodoPedido(tmp);
        *tamanhoString += getTamanhoPedidoStr(pedidoAtual);
        (*tamanhoString) ++; //Somar um ao tamanho por causa do \n 
        tmp = getProxNodo(tmp);
    }

    char* str = malloc(sizeof(char) * (*tamanhoString)); //aloca uma string com espaço para o pedido todo em forma de string
    str[0] = '\0';
    while (aux != NULL) {
        PEDIDO pedidoAtual = getNodoPedido(aux);
        char* pedidoAtualStr = getPedidoStr(pedidoAtual);
        strcat(str, pedidoAtualStr);
        strcat(str, "\n"); //fazer paragrafo para o proximo pedido estar noutra linha
        aux = getProxNodo(aux);
    }
    return str;
}

/*
Dado um array com espaco suficiente para todas as transformacoes, e o gestor de pedidos,
preenche o array com a utilizaçao de cada transformacao
*/
void createAtualArray(GESTOR_PEDIDOS gp) {
    int* atualArray = gp->atual;
    int ntransf = 7, i;

    //inicializa o array atual como tendo tudo zeros.
    for (i = 0; i < ntransf; i++) {
        atualArray[i] = 0;
    }

    NODO aux = gp->pedidosExecucao;
    while (aux != NULL) {
        //Para cada pedido no conjunto de pedidos em execucao, incrementa no atualArray a utilizaçao de transformaçoes do pedido
        PEDIDO pedidoAtual = getNodoPedido(aux);
        changeOcorrenciasTransformacoes(atualArray, pedidoAtual, 1);
        aux = getProxNodo(aux);
    }
}

void freeGestor(GESTOR_PEDIDOS g){
    free(g);
    //dar free ao pointer para o gestor ja da free aos arrays pedidosEmExecucao e pedidosEmEspera,
    //dado que a alocação destes arrays foi tada feita ao mesmo tempo que se alocou o gestor de pedidos.
}

int* getMaximo(GESTOR_PEDIDOS gp) {
    return gp->maximo;
}

int* getAtual(GESTOR_PEDIDOS gp) {
    return gp->atual;
}