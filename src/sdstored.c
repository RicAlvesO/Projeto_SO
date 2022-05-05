// Programa servidor
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h> //mkfifo
#include <sys/wait.h> //waitpid e WNOHANG
#include "../libs/servidor.h"
#include "../libs/pedido.h"
#include "../libs/gestor_pedidos.h"

// Verify If Bin Files Exists
int verifyBinFiles(char* path){
    int flag = 1;
    char *bins[]={"/bcompress","/bdecompress","/decrypt","/encrypt","/gcompress","/gdecompress","/nop"};
    for(int i=0; i<7; i++){
    
        // Create file destinations
        char str[(sizeof(bins[i]) + sizeof(path)) / sizeof(char)] = "";
        if(path[strlen(path)-1]=="/")path[strlen(path)-1]="\0";
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
    int fd, i=1;
    
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

    int pedidos[2];
    int serverProdutorGestorConsumidor[2];
    int serverConsumidorGestorProdutor[2];
    pipe(pedidos);
    pipe(serverProdutorGestorConsumidor);
    pipe(serverConsumidorGestorProdutor);
    
    if (mkfifo(fifo_geral,0666) == -1) {
        write(1,"[SERVIDOR]: fifo ja criado\n", 28);
    }
    
    switch(fork()) {
        case -1:
            printf("erro\n");
            break;
        case 0:
            //codigo filho, gestor de pedidos
            close(pedidos[1]);
            close(serverProdutorGestorConsumidor[1]);
            close(serverConsumidorGestorProdutor[0]);
            int fifoauxleitura = open(fifo_geral, O_WRONLY);
            int pid, status, x;
            PEDIDO pedido;
            GESTOR_PEDIDOS gp = createGestorPedidos(argv[1], argv[2]);
            //codigo filho,  gere os pedidos, recebe pedidos novos do pai
            int retval1 = fcntl(pedidos[0], F_SETFL, fcntl(pedidos[0], F_GETFL) | O_NONBLOCK);
            int retval2 = fcntl(serverProdutorGestorConsumidor[0], F_SETFL, fcntl(serverProdutorGestorConsumidor[0], F_GETFL) | O_NONBLOCK);
            printf("Ret1 from fcntl: %d\n", retval1);
            printf("Ret2 from fcntl: %d\n", retval2);
            /*
            Este ciclo while serve para gerir os pedidos
            Ele a todo o momento verifica se algum pedido acabou (waitpid), e se de facto acabou ele atualiza
            */
            while(1) {
                while ((pid = waitpid(-1,&status,WNOHANG)) > 0) { //enquanto houverem pedidos terminados
                    /*
                    O corpo deste ciclo corre por cada processo filho que termina, ou seja, por cada pedido que termina.
                    Dado que um pedido acaba de ser terminado, é preciso remover o pedido do conjunto de pedidos a serem executados
                    E é preciso verificar se algum pedido na fila de espera pode começar a ser executado (tryInserirPedido).
                    */
                    removerPedido(gp, pid); //remove do gestor de pedidos o pedido cujo processo acabou
                    printf("pedido removido pid - %d\n", pid);
                    createAtualArray(gp); //atualiza o array atual
                    tryInserirPedido(gp);
                    
                }
                
                while ((pedido = readPedido(pedidos[0])) != NULL) { //enquanto houver pedidos
                    printf("Leu um pedido\n");
                    setPedidoNth(pedido, i); i++;
                    openClienteFd(pedido);
                    inserirPedido(gp, pedido);
                    //printPedido(pedido);
                }
                if((read(serverProdutorGestorConsumidor[0], &x, sizeof(int))) > 0) { //se o processo pai quiser o estado do servidor, manda o estado do servidor
                    //Se não houver nada para ler no pipe, este devolve -1 e continua execuçao, dado que o read neste pipe é nao bloqueador
                    SERVER serverAtual = createServerFromGestor(gp); //cria o estado atual do servidor
                    writeServer(serverAtual, serverConsumidorGestorProdutor[1]);
                    freeServer(&serverAtual);
                }

                usleep(50000); //espera 50 milissegundos
            }
            break;
        default:
            //codigo pai, vai receber linhas de input de clientes do fifo_geral

            close(pedidos[0]); //pai so vai escrever os pedidos para o gestor de pedidos
            close(serverProdutorGestorConsumidor[0]);
            close(serverConsumidorGestorProdutor[1]);

            char buffer[128]; //path para o fifo
            
            int fifo = open(fifo_geral, O_RDONLY); //o servidor so precisa de ler linhas do fifo geral.
            
            while (read(fifo, buffer, 128) > 0) { //le os paths dos fifos privados de cada cliente
                
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
                    write(serverProdutorGestorConsumidor[1], &x, sizeof(int)); //manda qualquer coisa para o gestor para ele saber que o processo pai precisa do estado do servidor
                    SERVER server = readServer(serverConsumidorGestorProdutor[0]); //le o servidor no estado atual do pipe para onde o gestor escreveu
                    writeServer(server, fdEscrita);
                    freeServer(&server);
                } else if (x==1) { // -> cliente vai enviar pedido
                    PEDIDO pedido = readPedido(fdLeitura);
                    setClienteFifoStr(pedido, fifoEscrita);
                    writePedido(pedido,pedidos[1]);
                    free_pedido(pedido);
                }
                //close(fdLeitura);
                //close(fdEscrita);
            }

            break;
    }
    return 1;
}