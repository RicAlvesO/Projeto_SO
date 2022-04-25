#include "../libs/pedido.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define MAX_TRANSFORMATION_SIZE 20

struct pedido{
    int pid; //pid do processo que esta a realizar o pedido.
    /*
    int usadas[10]; -> guarda a quantidade de cada transformação que o pedido usa.
    Por exemplo se o pedido usa 3 nops, o indice respetivo da operação nop teria um 3, e o resto das posiçoes teriam 0.
    */
    int prioridade;
    char inputPath[128];
    char outputPath[128];
    int ntransformacoes; //numero de transformacoes
    int *transformacoes; //nao sabemos quantas transformações existem, mas cada uma tem 20 chars de tamanho no maximo
};

PEDIDO createEmptyPedido() {
    PEDIDO pedido = malloc(sizeof(struct pedido));
    return pedido;
}

PEDIDO createPedido(int prio, char* inputPath, char* outputPath, int ntransf, char **transformacoes) {
    PEDIDO pedido = malloc(sizeof(struct pedido));
    pedido->transformacoes=malloc(sizeof(int)*ntransf);
    pedido->prioridade = prio; //prioridade do pedido
    pedido->ntransformacoes = ntransf; //quantidade de transformacoes
    strcpy(pedido->inputPath, inputPath);
    strcpy(pedido->outputPath, outputPath);
    for (int i = 0; i < ntransf; i++) {
        pedido->transformacoes[i]= transf_to_code(transformacoes[i]);
    }
    return pedido;
}

void writePedido(PEDIDO pedido, int fd) {
    //if (write(fd, pedido, sizeof(struct pedido)) != sizeof(struct pedido)) {
    //    write(2, "nao conseguiu escrever no fifo: ", 33);
    //}
    write(fd, pedido, sizeof(*pedido) + pedido->ntransformacoes * sizeof(pedido->transformacoes[0]));
}

//le um pedido do file descriptor fd e mete na struct pedido. Devolve o numero de bytes lidos
//int readPedido(PEDIDO pedido, int fd) {
//    int ret;
//    if ((ret = read(fd, pedido, sizeof(struct pedido))) != sizeof(struct pedido)) {
//        //write(2, "nao conseguiu ler no fifo: ", 28);
//        return ret;
//    }
//    return 2;
//}

PEDIDO readPedido(int fd) {
    int ret;
    PEDIDO pedido1 = createEmptyPedido();
    PEDIDO pedido2;
    if ((ret = read(fd, pedido1, sizeof(struct pedido))) != sizeof(struct pedido)) {
        return NULL; //nao ha pedidos para ler
    }
    //agora o pedido2 tem a informaçao de quantas transformaçoes ha, logo consegue alocar espaço suficiente para elas
    pedido2 = malloc(sizeof(struct pedido) + pedido1->ntransformacoes * sizeof(pedido1->transformacoes[0]));
    *pedido2 = *pedido1;
    if (read(fd, pedido2->transformacoes, pedido2->ntransformacoes * sizeof(pedido2->transformacoes[0])) != pedido2->ntransformacoes * sizeof(pedido2->transformacoes[0])) {
        write(2, "nao conseguiu ler as transformacoes ", 37);
    }
    return pedido2;
}

void printPedido(PEDIDO p) {
    printf("A printar pedido\n");
    printf("Prio - %d\n", p->prioridade);
    printf("Input path - %s\n", p->inputPath);
    printf("Output path - %s\n", p->outputPath);
    printf("nTransf - %d\n", p->ntransformacoes);
    for (int i = 0; i<p->ntransformacoes; i++) {
        printf("Transformacao - %s\n", code_to_transf(p->transformacoes[i]));
    }
}

char* getPedidoStr(PEDIDO p) {
    int tamanhoStr = 0, i;
    tamanhoStr += strlen("proc-file") + 1; //tamanho do proc-file mais o espaço
    tamanhoStr += 2; //a prioridade e o espaço
    tamanhoStr += strlen(p->inputPath) + 1; //tamanho do inputPath mais o espaço
    tamanhoStr += strlen(p->outputPath) + 1; //tamanho do outputPath mais o espaço

    for (i=0; i<p->ntransformacoes; i++) {
        tamanhoStr += strlen(p->transformacoes[i]); //tamanho do argumento
        tamanhoStr ++; //o tamanho do espaço
    }
    char prio[2];
    sprintf(prio, "%d",p->prioridade);

    char* str = malloc(sizeof(char) * tamanhoStr);
    str[0] = '\0';
    strcat(str, "proc-file ");
    strcat(str, prio); strcat(str, " ");
    strcat(str, p->inputPath); strcat(str, " ");
    strcat(str, p->outputPath); strcat(str, " ");
    for(i=0; i<p->ntransformacoes; i++) {
        strcat(str, code_to_transf(p->transformacoes[i]));
        strcat(str, " ");
    }
    return str;
}

int getNTransformacoes(PEDIDO p) {
    return p->ntransformacoes;
}

//retorna o numero de ocorrencias de uma certa transformação num pedido
int ocorrenciasTransformacao(PEDIDO p, int transf) {
    int sum=0, ntransf = p->ntransformacoes, i;

    for (i=0; i<ntransf; i++) {
        if (transf==p->transformacoes[i]) sum++;
    }
    return sum;
}

/*
 * Executa o pedido p no processo atual
 *
 * Para executar o pedido atraves do codigo da transformacao
 * usar code_to_transf que retorna string da transformacao
 */
void executarPedido(PEDIDO p) {
    int pid;
    
    if ((pid = fork()) == 0) {
        /*
        Codigo do processo filho, que vai executar o pedido
        */
        printf("[PEDIDO]: Vou dormir\n");
        sleep(3);
        printf("[PEDIDO]: Dormi\n");
        _exit(0);
    } else {
        /*
        Codigo do processo pai, que vai atribuir ao pedido o pid do processo que o vai executar.
        */
        setPid(p,pid);
    }

}

void setPid(PEDIDO p,int pid) {
    p->pid = pid;
}

// Traduz transformacoes para o seu codigo de inteiro
int transf_to_code(char* transf)
{
    if (strcasecmp(transf, "nop") == 0)
    {
        return NOP;
    }
    else if (strcasecmp(transf, "gdecompress") == 0)
    {
        return GDE;
    }
    else if (strcasecmp(transf, "gcompress") == 0)
    {
        return GCO;
    }
    else if (strcasecmp(transf, "encrypt") == 0)
    {
        return ENC;
    }
    else if (strcasecmp(transf, "decrypt") == 0)
    {
        return DEC;
    }
    else if (strcasecmp(transf, "bdecompress") == 0)
    {
        return BDE;
    }
    else if (strcasecmp(transf, "bcompress") == 0)
    {
        return BCO;
    }
    else
    {
        return ERR;
    }
}

// Traduz codigos para a reschar *code_to_transf(int cd)petiva transformação
char *code_to_transf(int cd)
{
    switch (cd)
    {
    case NOP:
        return "nop";
        break;
    case GDE:
        return "gdecompress";
        break;
    case GCO:
        return "gcompress";
        break;
    case ENC:
        return "encrypt";
        break;
    case DEC:
        return "decopryt";
        break;
    case BDE:
        return "bdecompress";
        break;
    case BCO:
        return "bcompress";
        break;
    default:
        return EOF; // ERROR TRANSF NOT FOUND
        break;
    }
}

void free_pedido(PEDIDO p){
    free(p->transformacoes);
    free(p);
}