#include "pedido.h"

typedef struct gestor_pedidos * GESTOR_PEDIDOS;

GESTOR_PEDIDOS createGestorPedidos(int pedidosMax, char* config_file);
void inserirPedido(GESTOR_PEDIDOS gp, PEDIDO p);