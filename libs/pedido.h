
typedef struct pedido *PEDIDO;

PEDIDO createEmptyPedido();
PEDIDO createPedido(int prio, char* inputPath, char* outputPath, int ntransf, char **transformacoes);
void writePedido(PEDIDO pedido, int fd);
PEDIDO readPedido(int fd);
void printPedido(PEDIDO p);
char* getPedidoStr(PEDIDO p);
int getNTransformacoes(PEDIDO p);
int ocorrenciasTransformacao(PEDIDO p, char* transf);
void executarPedido(PEDIDO p);
void setPid(PEDIDO p,int pid);