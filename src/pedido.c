#include "../libs/pedido.h"
#include "../libs/funcoes.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

#define MAX_TRANSFORMATION_SIZE 20

struct pedido{
    int pid; //pid do processo que esta a realizar o pedido.
    int nth; //quantos pedidos fizeram antes deste + 1
    /*
    int usadas[10]; -> guarda a quantidade de cada transformação que o pedido usa.
    Por exemplo se o pedido usa 3 nops, o indice respetivo da operação nop teria um 3, e o resto das posiçoes teriam 0.
    */
    int prioridade;
    char inputPath[128];
    char outputPath[128];
    char clienteFifoStr[128]; //string que contem o caminho para o fifo de escrita de onde o cliente vai ler
    int fdCliente; //file descriptor do cliente
    int ntransformacoes; //numero de transformacoes
    int *transformacoes; //nao sabemos quantas transformações existem, mas cada uma tem 20 chars de tamanho no maximo
};

PEDIDO createEmptyPedido() {
    PEDIDO pedido = malloc(sizeof(struct pedido));
    //pedido->transformacoes = (int*) pedido + sizeof(struct pedido);
    return pedido;
}

PEDIDO createPedido(int prio, char* inputPath, char* outputPath, int ntransf, char **transformacoes) {
    PEDIDO pedido = malloc(sizeof(struct pedido) + sizeof(int)*ntransf);
    pedido->transformacoes = (int*) (((char*)pedido) + sizeof(struct pedido)); //coloca o apontador no sitio onde vao ficar as transformacoes
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
    if (write(fd, pedido, sizeof(*pedido) + pedido->ntransformacoes * sizeof(pedido->transformacoes[0])) != (sizeof(*pedido) + pedido->ntransformacoes * sizeof(pedido->transformacoes[0]))) {
        write(2, "nao conseguiu escrever pedido\n", 31);
    }
}

PEDIDO readPedido(int fd) {
    int ret;
    PEDIDO pedido1 = createEmptyPedido();
    if ((ret = read(fd, pedido1, sizeof(struct pedido))) != sizeof(struct pedido)) {
        free_pedido(pedido1);
        return NULL; //nao ha pedidos para ler
    }
    //agora o pedido2 tem a informaçao de quantas transformaçoes ha, logo consegue alocar espaço suficiente para elas
    PEDIDO pedido2 = malloc(sizeof(struct pedido) + pedido1->ntransformacoes * sizeof(pedido1->transformacoes[0]));
    *pedido2 = *pedido1;
    free_pedido(pedido1);
    pedido2->transformacoes = (int*) (((char*)pedido2) + sizeof(struct pedido));
    if ((ret = read(fd, pedido2->transformacoes, pedido2->ntransformacoes * sizeof(pedido2->transformacoes[0]))) != pedido2->ntransformacoes * sizeof(pedido2->transformacoes[0])) {
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

/*
Recebe um pedido e devolve o tamanho do pedido representado em string
*/
int getTamanhoPedidoStr(PEDIDO p) {
    int tamanhoStr = 0, i;
    tamanhoStr += 8 + (p->nth < 10 ? 1 : 2); //'task #x: ' sao 9 ou 10 caracteres dependendo se o x é de 1-9 ou 10 pra cima
    tamanhoStr += strlen("proc-file") + 1; //tamanho do proc-file mais o espaço
    tamanhoStr += 2; //a prioridade e o espaço
    tamanhoStr += strlen(p->inputPath) + 1; //tamanho do inputPath mais o espaço
    tamanhoStr += strlen(p->outputPath) + 1; //tamanho do outputPath mais o espaço

    for (i=0; i<p->ntransformacoes; i++) {
        tamanhoStr += strlen(code_to_transf(p->transformacoes[i])); //tamanho do argumento
        tamanhoStr ++; //o tamanho do espaço
    }
    return tamanhoStr;
}

char* getPedidoStr(PEDIDO p) {

    int tamanhoStr = getTamanhoPedidoStr(p), i;
    char* str = malloc(sizeof(char) * tamanhoStr); //aloca uma string com espaço para o pedido todo em forma de string
    
    char numeroEmString[3];

    str[0] = '\0';
    sprintf(numeroEmString, "%d",p->nth);
    strcat(str, "task #"); strcat(str, numeroEmString); strcat(str, ": "); //'task #x: '
    strcat(str, "proc-file "); //'proc-file '
    sprintf(numeroEmString, "%d",p->prioridade);
    strcat(str, numeroEmString); strcat(str, " "); //'x '
    strcat(str, p->inputPath); strcat(str, " "); //'caminho_entrada '
    strcat(str, p->outputPath); strcat(str, " "); //'caminho_saida '
    for(i=0; i<p->ntransformacoes; i++) {
        strcat(str, code_to_transf(p->transformacoes[i]));
        strcat(str, " ");
    }
    return str;
}

/*
Recebe um array de pedidos, o tamanho do array, e devolve uma string com todos os pedidos dentro.
Tambem mete no conteudo de tamanhoStr o tamanho da string toda.
*/

int getNTransformacoes(PEDIDO p) {
    return p->ntransformacoes;
}

int getPrioridade(PEDIDO p) {
    return p->prioridade;
}

int getPidPedido(PEDIDO p) {
    return p->pid;
}

//retorna o numero de ocorrencias de uma certa transformação num pedido
int ocorrenciasTransformacao(PEDIDO p, int transf) {
    int sum=0, ntransf = p->ntransformacoes, i;

    for (i=0; i<ntransf; i++) {
        if (transf==p->transformacoes[i]) sum++;
    }
    return sum;
}

void setPid(PEDIDO p,int pid) {
    p->pid = pid;
}

void setPedidoNth(PEDIDO p, int nth) {
    p->nth = nth;
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
        return "decrypt";
        break;
    case BDE:
        return "bdecompress";
        break;
    case BCO:
        return "bcompress";
        break;
    default:
        return "\0"; // ERROR TRANSF NOT FOUND
        break;
    }
}

void free_pedido(PEDIDO p){
    //apenas da-se free ao pointer para o pedido, dado que a alocaçao do array transformacoes esta contida na alocaçao do pedido
    free(p);
}

void setClienteFifoStr(PEDIDO p, char* fifoPath) {
    strcpy(p->clienteFifoStr, fifoPath);
}

void openClienteFd(PEDIDO p) {
    p->fdCliente = open(p->clienteFifoStr, O_WRONLY);
    //Assumindo que o pedido ja tem escrito o caminho para o fifo do cliente,
    //abre o fifo em modo escrita para mandar as informaçoes necessarias privadamente para o cliente.
}

void alertPedidoEmEspera(PEDIDO p) {
    //assume que o file descriptor do cliente esta aberto
    write(p->fdCliente, "pending\n",8);
}

void alertPedidoInserido(PEDIDO p) {
    //assume que o file descriptor do cliente esta aberto
    write(p->fdCliente, "processing\n", 11);
}

void alertPedidoConcluido(PEDIDO p, int bytesIn, int bytesOut) {
    //assume que o file descriptor do cliente esta aberto
    char str[128];
    sprintf(str, "concluded (bytes-input: %d, bytes-output: %d)\n", bytesIn, bytesOut);
    write(p->fdCliente, str, strlen(str));
    write(p->fdCliente, "end\n", 4);
}

PEDIDO encontrarPedido(PEDIDO* pedidos, int N, int pid) {
    int i;
    for (i=0; i<N; i++) {
        PEDIDO p = pedidos[i];
        if (p->pid == pid) {
            return p;
        }
    }
    return NULL; //nenhum pedido na lista estava a ser realizado pelo processo de id 'pid', logo retorna NULL
}

/*
Recebe um array de transformacoes atuais, e um pedido, e incrementa no array as transformacoes que o pedido usa.
Para cada transformacao, incrementa o indice da mesma no array.
*/
void addOcorrenciasTransformacoes(int* atualArray, PEDIDO p) {
    int ntransf = p->ntransformacoes, i;
    int* transfs = p->transformacoes;

    for (i = 0; i < ntransf; i++) {
        int transformacaoAtual = transfs[i];
        atualArray[transformacaoAtual]++;
    }
}

int executaTransformacao(char* path, int transformacao) {
    //assume-se que a transformacao é so uma palavra
    char executavel[128];
    char* exec_args[2];
    int exec_ret = 0;
    executavel[0] = '\0';
    strncat(executavel, path, 128); //assume-se que o path acaba em /
    strcat(executavel, code_to_transf(transformacao)); //converte a transformacao em string e concateneia-la
    exec_args[0] = executavel;
    exec_args[1] = NULL;

    exec_ret = execvp(exec_args[0], exec_args);

    return exec_ret;
}

void executarPedido(PEDIDO p, char* folder_path) {

    int pid;

    if ((pid = fork()) == 0) {
        int ntransf = p->ntransformacoes;
        int* transf = p->transformacoes;
        int inputFd;
        int outputFd;
        if ((inputFd = open(p->inputPath, O_RDONLY, 0666)) == -1) {
            write(2, "Erro a abrir ficheiro input\n",29);
            perror("erro: ");
        }
        if ((outputFd = open(p->outputPath, O_WRONLY | O_CREAT, 0666)) == -1) {
            write(2, "Erro a abrir ficheiro output\n", 30);
        }
        int pipes[ntransf-1][2];
        int status[ntransf];
        int i;

        // criar os pipes conforme o número de comandos
        for (i = 0; i < ntransf-1; i++) {
            if (pipe(pipes[i]) == -1) {
                perror("Pipe não foi criado");
                return;
            }
        }

        if (ntransf == 1) { //so ha uma transformacao
            switch(fork()) {
                    case -1:
                        perror("Fork não foi efetuado");
                        return;
                    case 0:
                        dup2(inputFd,0);
                        close(inputFd);

                        dup2(outputFd,1);
                        close(outputFd);

                        executaTransformacao(folder_path,transf[i]);

                        _exit(0);
                        break;
                    default:
                        //codigo pai
                        break;
            }
        } else {
            for (i = 0; i < ntransf; i++) {
                if (i == 0) {
                    switch(fork()) {
                        case -1:
                            perror("Fork não foi efetuado");
                            return;
                        case 0:
                            // codigo do filho 0

                            close(pipes[i][0]);

                            dup2(pipes[i][1],1);
                            close(pipes[i][1]);

                            dup2(inputFd,0);
                            close(inputFd);

                            executaTransformacao(folder_path,transf[i]);

                            _exit(0);

                        default:
                            close(pipes[i][1]);
                    }
                }
                else if (i == ntransf-1) {
                    switch(fork()) {
                        case -1:
                            perror("Fork não foi efetuado");
                            return;
                        case 0:
                            // codigo do filho n-1

                            //if(close(p[i-1][1]) != 0) perror("close");

                            dup2(pipes[i-1][0],0);
                            close(pipes[i-1][0]);

                            dup2(outputFd,1);
                            close(outputFd);

                            executaTransformacao(folder_path,transf[i]);

                            _exit(0);

                        default:
                            close(pipes[i-1][0]);
                    }
                }
                else {
                    switch(fork()) {
                        case -1:
                            perror("Fork não foi efetuado");
                            return;
                        case 0:
                            // codigo do filho i

                            //if(close(p[i-1][1]) != 0){perror("close");}
                            close(pipes[i][0]);

                            dup2(pipes[i][1],1);
                            close(pipes[i][1]);

                            dup2(pipes[i-1][0],0);
                            close(pipes[i-1][0]);

                            executaTransformacao(folder_path,transf[i]);

                            _exit(0);

                        default:
                            close(pipes[i-1][0]);
                            close(pipes[i][1]);
                    }
                }
            }
        }

        

        for (i = 0; i < ntransf; i++) {
            wait(&status[i]);

            if (WIFEXITED(status[i])) {
                //printf("[PAI]: filho terminou com %d\n", WEXITSTATUS(status[i]));
            }
        }
        int bytesIn = contarBytes(p->inputPath); //bytes input
        int bytesOut = contarBytes(p->outputPath); //bytes output
        sleep(3);
        close(inputFd);
        close(outputFd);
        alertPedidoConcluido(p, bytesIn, bytesOut);
        _exit(0);
    } else {
        //enquanto o filho executa o pedido, o pai continua a execuçao, atribuindo o pid do processo ao pedido que ele executa.
        setPid(p,pid);
    }
}