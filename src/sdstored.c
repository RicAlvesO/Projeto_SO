// Programa servidor
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h> //mkfifo
#include <sys/wait.h> //waitpid e WNOHANG
#include "../libs/servidor.h"
#include "../libs/pedido.h"
#include "../libs/gestor_pedidos.h"

volatile sig_atomic_t terminandoGraciosamente = 0;

void term(int signum)
{
   printf("Terminando Graciosamente o Servidor, vou esperar que os pedidos recebidos acabem!\n");
   terminandoGraciosamente = 1;
   //signal(SIGTERM, &term);
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
    char* fifo_geral = "fifos/fifo_geral";
    int i=1, status, pid;
    char s1[50],s2[50];
    sprintf(s1,"[SERVIDOR]: (pid)%d\n",getpid());
    write(2,s1,strlen(s1));
    
    signal(SIGTERM, term);
    
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
    //printServerStatus(server);
    
    if (mkfifo(fifo_geral,0666) == -1) {
        write(1,"[SERVIDOR]: fifo ja criado\n", 28);
    }

    GESTOR_PEDIDOS gp = createGestorPedidos(argv[1], argv[2]);
    char buffer[128]; //path para o fifo
    int fifo = open(fifo_geral, O_RDONLY | O_NONBLOCK); //o servidor so precisa de ler linhas do fifo geral.  | O_NONBLOCK
    while ((!terminandoGraciosamente) || (!gestorIsEmpty(gp))) { //enquanto nao é pra terminar e ainda ha pedidos por fazer
        while ((!terminandoGraciosamente) && read(fifo, buffer, 128) > 0) { //le os paths dos fifos privados de cada cliente
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
                write(2,"[SERVIDOR]: Pedido lido\n",25);
                setClienteFifoStr(pedido, fifoEscrita);
                setPedidoNth(pedido, i); i++;
                openClienteFd(pedido);
                inserirPedido(gp, pedido);
            }
            close(fdLeitura);
            close(fdEscrita);
        }

        //wait nao bloqueador para esperar por pedidos
        while ((pid = waitpid(-1,&status,WNOHANG)) > 0) { //enquanto houverem pedidos terminados
            /*
            O corpo deste ciclo corre por cada processo filho que termina, ou seja, por cada pedido que termina.
            Dado que um pedido acaba de ser terminado, é preciso remover o pedido do conjunto de pedidos a serem executados
            E é preciso verificar se algum pedido na fila de espera pode começar a ser executado (tryInserirPedido).
            */
            removerPedido(gp, pid); //remove do gestor de pedidos o pedido cujo processo acabou
            sprintf(s2,"[SERVIDOR]: Pedido removido (pid)%d\n",getpid());
            write(2,s2,strlen(s2));
            
            createAtualArray(gp); //atualiza o array atual
            while(tryInserirPedido(gp)); //tenta inserir na fila de execucao pedidos que estao na fila de espera.
        }
    }
}