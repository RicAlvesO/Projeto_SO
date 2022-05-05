#include "../libs/ll_pedidos.h"
#include "../libs/pedido.h"
#include <stddef.h>
#include <stdlib.h>


struct nodo {
    PEDIDO pedido;
    NODO prox;
};

NODO createNodo(PEDIDO p) {
    NODO nodo = malloc(sizeof(struct nodo));
    nodo->pedido = p;
    nodo->prox = NULL;
    return nodo;
}

/*
Esta função insere um NODO na lista ligada, tendo em conta a prioridade do nodo.
Assim, assumindo que a lista ligada que é recebida como argumento já esta ordenada por prioridade,
o nodo é inserido na lista ligada, mantendo a ordem da mesma.
*/
void inserirNodo(NODO* cabeca_ll, NODO nodo) {
    int prioPedido = getPrioridade(nodo->pedido); //prioridade do nodo a inserir;
    while ((*cabeca_ll != NULL) && (getPrioridade((*cabeca_ll)->pedido) > prioPedido)) {
        cabeca_ll = &((*cabeca_ll)->prox);
    }
    //A este ponto, cabeca_ll aponta para o sitio onde é suposto o nodo ficar.
    //Entao insere-se o nodo aqui e "empurra-se" para baixo a lista ligada.
    nodo->prox = *cabeca_ll;
    *cabeca_ll = nodo;
}

/*
Esta função recebe uma lista ligada ordenada por prioridade dos pedidos, e o pid do pedido a ser removido,
e simplesmente remove o tal pedido, mantendo a ordem da lista.
Se nao encontrar nenhum pedido com o pid dado, não altera a lista ligada.
*/
void removerNodo(NODO* cabeca_ll, int pidPedido) {

    while ((*cabeca_ll != NULL) && (getPidPedido((*cabeca_ll)->pedido) != pidPedido)) {
        cabeca_ll = &((*cabeca_ll)->prox);
    }

    if (*cabeca_ll != NULL) {
        NODO nodoRemovido = *cabeca_ll;
        *cabeca_ll = (*cabeca_ll)->prox;
        nodoRemovido->prox = NULL;
        //freeNodo(nodoRemovido);
    }
}

/*
Recebe um nodo e da free apenas ao nodo dado (nao da free aos nodos seguintes);
*/
void freeNodo(NODO nodo) {
    free_pedido(nodo->pedido);
    free(nodo);
}

/*
Recebe uma lista ligada e remove todos os nodos
*/
void freeListaLigada(NODO* nodo) {
    if (*nodo != NULL) {
        freeListaLigada(&((*nodo)->prox));
        freeNodo(*nodo);
        *nodo = NULL;
    }
}

PEDIDO getNodoPedido(NODO nodo) {
    return nodo->pedido;
}

NODO getProxNodo(NODO nodo) {
    return nodo->prox;
}