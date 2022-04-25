#include "gestor_pedidos.h"

typedef struct server * SERVER;

SERVER createServerFromGestor(GESTOR_PEDIDOS gp);
SERVER createServer(char* config_file);
SERVER readServer(int fd);
void writeServer(SERVER server, int fd);
void freeServer(SERVER* server);
void printServerStatus(SERVER server);