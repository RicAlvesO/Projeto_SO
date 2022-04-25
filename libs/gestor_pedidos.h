#include "pedido.h"

typedef struct gestor_pedidos * GESTOR_PEDIDOS;

GESTOR_PEDIDOS createGestorPedidos( char* config_file);
void inserirPedido(GESTOR_PEDIDOS gp, PEDIDO p);
void removerPedido(GESTOR_PEDIDOS gp, int pidPedido);
void freeGestor(GESTOR_PEDIDOS gp);