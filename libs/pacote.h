

typedef struct pacote * PACOTE;

PACOTE createEmptyPacote();
PACOTE createServerStatusPacote(pid_t pid);
PACOTE createSendingPedidoPacote(pid_t pid);
PACOTE createPedidoFinishedPacote(pid_t pid);
void writePacote(PACOTE pacote, int fd);
PACOTE readPacote(int fd);
int isServerStatusRequestPacote(PACOTE pacote);
int isSendingPedidoPacote(PACOTE pacote);
int isPedidoFinishedPacote(PACOTE pacote);
pid_t getPacotePid(PACOTE pacote);
void freePacote(PACOTE* pacote);