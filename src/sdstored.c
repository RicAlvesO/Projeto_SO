// Programa servidor
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h> //mkfifo
#include <stdlib.h>
#include <sys/wait.h> //waitpid e WNOHANG
#include "../libs/servidor.h"
#include "../libs/pedido.h"
#include "../libs/gestor_pedidos.h"

int terminandoGraciosamente = 0;
int filhosPorTerminar = 0;
int clientesPorReceber = 0;
pid_t pid_gestor_pedidos;

void sig_chld_handler(int signum) { //handler de filhos por terminar
    filhosPorTerminar++;
}

void sig_usr1_handler(int signum) { //handler de clientes por receber
    clientesPorReceber++;
}

void terminar_pai(int signum) {
   terminandoGraciosamente = 1;
   kill(pid_gestor_pedidos, SIGTERM);
   exit(0);
}

void terminar_gestor(int signum) {
   terminandoGraciosamente = 1;
}

// Verify If Bin Files Exists
int verifyBinFiles(char* path){
    int flag = 1;
    char *bins[]={"/bcompress","/bdecompress","/decrypt","/encrypt","/gcompress","/gdecompress","/nop"};
    for(int i=0; i<7; i++){
    
        // Create file destinations
        char str[(sizeof(bins[i]) + sizeof(path)) / sizeof(char)] = "";
        if(path[strlen(path)-1]=='/') path[strlen(path)-1]='\0';
        strcat(str, path);
        strcat(str, bins[i]);
        
        // Verify file existance
        if (access(str, F_OK) == -1){
            
            // Make ERROR message
            char error[(sizeof(bins[i])/8) + 12] = "";
            strcat(error, bins[i]);
            strcat(error, "NOT FOUND!\n");
            
            // Print ERROR message
            write(2, error, strlen(error));
            flag=0;
        }
    }
    return flag;
}

// Main Function
int main(int argc, char* argv[]) {
    // Argument Verifications
    if (argc != 3){
        write(2, "NOT ENOUGH ARGUMENTS PROVIDED!\n", 32);
        return -1;
    }
    // Config File Verification
    if (access(argv[1], F_OK) == -1){
        write(2, "CONFIG FILE NOT FOUND!\n", 24);
        return -1;
    }
    //Bin Files Verification
    /*
    * Verifies the folder in the argument 
    * To work bins should be generated prior to using this program
    * To generate run `make` on libs folder
    */
    if(!verifyBinFiles(argv[2]))return -1;

    char s1[50];
    sprintf(s1, "[SERVIDOR]: pid: %d\n", getpid());
    write(1, s1, strlen(s1));
    
    char* fifo_geral = "fifos/fifo_geral";
    mkfifo(fifo_geral,0666);

    int i=1, status, pid, fd[2];
    pipe(fd);
    
    signal(SIGCHLD, sig_chld_handler);

    if ((pid = fork()) == 0) {
        close(fd[1]);
        signal(SIGTERM, terminar_gestor);
        signal(SIGUSR1, sig_usr1_handler);
        /*
        Filho
        Este processo é responsavel por gerir os pedidos. Ele adiciona pedidos que cheguem, e remove pedidos que acabem
        A variavel fifo simplesmente serve para manter o fifo geral aberto
        */
        int fifo = open(fifo_geral, O_WRONLY);
        GESTOR_PEDIDOS gp = createGestorPedidos(argv[1], argv[2]);
        char buffer[128]; //path para o fifo
        while ((!terminandoGraciosamente) || (!gestorIsEmpty(gp))) { //enquanto nao é pra terminar e ainda ha pedidos por fazer
            /*
            O gestor de pedidos constantemente entra num estado de pause, que apenas é desbloqueado quando chega
            um cliente, ou quando acaba um pedido, que são os dois pontos de sincronização com o gestor, ou seja,
            são os únicos dois momentos que o gestor precisa de ser alterado.
            Assim, enquanto não chegarem clientes, nem acabarem pedidos, este processo fica em espera, e não gasta recursos.
            */
            pause();

            while (clientesPorReceber > 0) {
                clientesPorReceber--;
                read(fd[0], buffer, sizeof(buffer));
                char fifoEscrita[128];
                char fifoLeitura[128];
                strcpy(fifoLeitura, buffer);
                strcpy(fifoEscrita, buffer);
                strcat(fifoLeitura, "-cliente_produtor");
                strcat(fifoEscrita, "-cliente_consumidor");
                int fdLeitura = open(fifoLeitura, O_RDONLY);
                int fdEscrita = open(fifoEscrita, O_WRONLY);
                int x=-1;
                read(fdLeitura, &x, sizeof(int));
                if (x==0) { // -> quer estado do servidor
                    SERVER server = createServerFromGestor(gp); //cria o estado do servidor atual
                    writeServer(server, fdEscrita); //manda servidor para o cliente
                    freeServer(&server);
                } else if (x==1) { // -> cliente vai enviar pedido
                    PEDIDO pedido = readPedido(fdLeitura);
                    setClienteFifoStr(pedido, fifoEscrita);
                    setPedidoNth(pedido, i); i++;
                    openClienteFd(pedido);
                    inserirPedido(gp, pedido);
                }
                close(fdLeitura);
                close(fdEscrita);
            }
    
            while (filhosPorTerminar > 0) {
                filhosPorTerminar--;
                pid = wait(&status);
                /*
                O corpo deste ciclo corre por cada processo filho que termina, ou seja, por cada pedido que termina.
                Dado que um pedido acaba de ser terminado, é preciso remover o pedido do conjunto de pedidos a serem executados
                E é preciso verificar se algum pedido na fila de espera pode começar a ser executado (tryInserirPedido).
                */
                removerPedido(gp, pid); //remove do gestor de pedidos o pedido cujo processo acabou
                
                createAtualArray(gp); //atualiza o array atual
                while(tryInserirPedido(gp)); //tenta inserir na fila de execucao pedidos que estao na fila de espera.
            }
        }
        close(fifo);
        close(fd[0]);
        write(1, "gestor terminou de executar\n",29);
        _exit(0);
    } else {
        close(fd[0]);
        signal(SIGTERM, terminar_pai);
        pid_gestor_pedidos = pid;
        /*
        Pai
        Este processo simplesmente le strings do fifo geral (para onde os clientes escrevem os seus paths),
        e reescreve a string para o processo gestor de pedidos, mandando um sinal para o mesmo, avisando que ha um
        cliente por receber
        pid guarda o pid do filho para mandar o sinal a avisar que falta tratar de um cliente
        No pipe fd, o pai é produtor porque manda para la os paths para os clientes, logo fecha fd[0]
        */
        int fifo = open(fifo_geral, O_RDONLY); //o servidor so precisa de ler linhas do fifo geral.  | O_NONBLOCK
        char buffer[128]; //path para o fifo
        while (read(fifo, buffer, 128) > 0 && (!terminandoGraciosamente)) {
            kill(pid, SIGUSR1); //avisa o gestor de pedidos que falta tratar de um cliente
            write(fd[1], buffer, sizeof(buffer));
        }
        wait(NULL);
        close(fifo);
        close(fd[1]);
    }
    return 0;
}