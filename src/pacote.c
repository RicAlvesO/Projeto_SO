#include <unistd.h>
#include <stdlib.h>
#include "../libs/pacote.h"

#define SERVER_STATUS_REQUEST 0
#define SENDING_PEDIDO 1
#define PEDIDO_FINISHED 2

struct pacote {
    pid_t pid;
    int tipo;
};

PACOTE createEmptyPacote() {
    PACOTE pacote = malloc(sizeof(struct pacote));
    return pacote;
}

PACOTE createServerStatusPacote(pid_t pid) {
    PACOTE pacote = malloc(sizeof(struct pacote));
    pacote->pid = pid;
    pacote->tipo = SERVER_STATUS_REQUEST;
    return pacote;
}

PACOTE createSendingPedidoPacote(pid_t pid) {
    PACOTE pacote = malloc(sizeof(struct pacote));
    pacote->pid = pid;
    pacote->tipo = SENDING_PEDIDO;
    return pacote;
}

PACOTE createPedidoFinishedPacote(pid_t pid) {
    PACOTE pacote = malloc(sizeof(struct pacote));
    pacote->pid = pid;
    pacote->tipo = PEDIDO_FINISHED;
    return pacote;
}

void writePacote(PACOTE pacote, int fd) {
    if (write(fd, pacote, sizeof(struct pacote)) != (sizeof(struct pacote))) {
        write(2, "[ERROR]: Nao conseguiu escrever o pacote\n", 42);
    }
}

PACOTE readPacote(int fd) {
    PACOTE p = malloc(sizeof(struct pacote));
    if ((read(fd, p, sizeof(struct pacote))) == sizeof(struct pacote)) {
        return p;
    } else {
        return NULL;
    }
}

int isServerStatusRequestPacote(PACOTE pacote) {
    return pacote->tipo == SERVER_STATUS_REQUEST;
}

int isSendingPedidoPacote(PACOTE pacote) {
    return pacote->tipo == SENDING_PEDIDO;
}

int isPedidoFinishedPacote(PACOTE pacote) {
    return pacote->tipo == PEDIDO_FINISHED;
}

pid_t getPacotePid(PACOTE pacote) {
    return pacote->pid;
}

void freePacote(PACOTE* pacote) {
    free(*pacote);
    *pacote = NULL;
}