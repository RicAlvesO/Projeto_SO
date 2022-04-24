
typedef struct pedido *PEDIDO;

PEDIDO createEmptyPedido();
PEDIDO createPedido(int prio, char* inputPath, char* outputPath, int ntransf, char **transformacoes);
void writePedido(PEDIDO pedido, int fd);
PEDIDO readPedido(int fd);
void printPedido(PEDIDO p);
