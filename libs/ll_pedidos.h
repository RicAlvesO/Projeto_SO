#include "pedido.h"

typedef struct nodo * NODO;

NODO createNodo(PEDIDO p);
void inserirNodo(NODO* cabeca_ll, NODO nodo);
NODO removerNodo(NODO* cabeca_ll, int pidPedido);
void freeNodo(NODO nodo);
void freeListaLigada(NODO* nodo);
PEDIDO getNodoPedido(NODO nodo);
NODO getProxNodo(NODO nodo);
PEDIDO getPedido(NODO nodo, pid_t pid);