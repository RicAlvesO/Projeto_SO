#include "pedido.h"

typedef struct gestor_pedidos * GESTOR_PEDIDOS;


GESTOR_PEDIDOS createGestorPedidos( char* config_file, char* transf_folder);
void inserirPedido(GESTOR_PEDIDOS gp, PEDIDO p);
void removerPedido(GESTOR_PEDIDOS gp, int pidPedido);
void createAtualArray(GESTOR_PEDIDOS gp);
void freeGestor(GESTOR_PEDIDOS gp);
int* getMaximo(GESTOR_PEDIDOS gp);
int* getAtual(GESTOR_PEDIDOS gp);
int getNPedidosEmExecucao(GESTOR_PEDIDOS gp);
PEDIDO* getPedidosEmExecucao(GESTOR_PEDIDOS gp);
char* getAllPedidosStr(GESTOR_PEDIDOS gp, int *tamanhoString);