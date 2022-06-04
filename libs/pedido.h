#include <unistd.h>

typedef struct pedido *PEDIDO;

#define ERR -1
#define NOP 0
#define GDE 1
#define GCO 2
#define ENC 3
#define DEC 4
#define BDE 5
#define BCO 6

PEDIDO createEmptyPedido();
PEDIDO createPedido(int prio, char* inputPath, char* outputPath, int ntransf, char **transformacoes);
void writePedido(PEDIDO pedido, int fd);
PEDIDO readPedido(int fd);
void printPedido(PEDIDO p);
int getTamanhoPedidoStr(PEDIDO p);
pid_t getPidPedido(PEDIDO p);
int getPrioridade(PEDIDO p);
char* getPedidoStr(PEDIDO p);
int getNTransformacoes(PEDIDO p);
int ocorrenciasTransformacao(PEDIDO p, int transf);
void setPedidoNth(PEDIDO p, int nth);
void setPid(PEDIDO p,int pid);
int transf_to_code(char *transf); 
char* code_to_transf(int cd);
void free_pedido(PEDIDO p);
void setClienteFifoStr(PEDIDO p, char* fifoPath);
void openClienteFd(PEDIDO p);
void alertPedidoEmEspera(PEDIDO p);
void alertPedidoInserido(PEDIDO p);
void alertPedidoConcluido(PEDIDO p, int bytesIn, int bytesOut);
PEDIDO encontrarPedido(PEDIDO* pedidos, int N, int pid);
void changeOcorrenciasTransformacoes(int* atualArray, PEDIDO p, int adicionar);
int executaTransformacao(char* path, int transformacao);
void executarPedido(PEDIDO p, char* folder_path);
void printPedidoInputPath(PEDIDO p);
void setPedidoFifoGeral(PEDIDO p, int fd);