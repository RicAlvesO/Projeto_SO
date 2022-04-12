

typedef struct server * SERVER;

SERVER createServer(char* config_file);
void printServerStatus(SERVER server);