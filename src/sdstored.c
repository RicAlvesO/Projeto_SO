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
            GESTOR_PEDIDOS gp = createGestorPedidos(argv[1]);
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
                    printf("Acabou um pedido\n");
                    removerPedido(gp, pid); //remove do gestor de pedidos o pedido cujo processo acabou
                }
                while ((pedido = readPedido(pedidos[0])) != NULL) { //enquanto houver pedidos
                    printf("Leu um pedido\n");
                    setPedidoNth(pedido, i); i++;
                    openClienteFd(pedido);
                    char* str = getPedidoStr(pedido);
                    printf("pedido - %s\n", str);
                    inserirPedido(gp, pedido);
                    //printPedido(pedido);
                }
                if((read(serverProdutorGestorConsumidor[0], &x, sizeof(int))) > 0) { //se o processo pai quiser o estado do servidor, manda o estado do servidor
                    //Se não houver nada para ler no pipe, este devolve -1 e continua execuçao, dado que o read neste pipe é nao bloqueador
                    SERVER serverAtual = createServerFromGestor(gp); //cria o estado atual do servidor
                    writeServer(serverAtual, serverConsumidorGestorProdutor[1]);
                    //freeServer(&serverAtual);
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
            //if (mkfifo(fifo_geral,0666) == -1) {
            //    write(1,"[SERVIDOR]: fifo ja criado\n", 28);
            //}
            /*
            Nesta seccao é aberto um descritor de escrita permanente auxiliar para o fifo_geral, de maneira a que este
            nao seja fechado quando um cliente acaba e fecha o seu descritor do fifo_geral.
            
            if (fork() == 0) {
                printf("AQUI1\n");
                _exit(0);
            }
            */    
            //int fifoaux = open(fifo_geral, O_RDONLY); //este fifo so serve para o read nao ler EOF.
            //Nao se usara o fifoaux.
            
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
                } else if (x==1) { // -> cliente vai enviar pedido
                    PEDIDO pedido = readPedido(fdLeitura);
                    setClienteFifoStr(pedido, fifoEscrita);
                    writePedido(pedido,pedidos[1]);
                }
                //close(fdLeitura);
                //close(fdEscrita);
            }

            break;
    }

    //switch(fork()) {
    //    case -1:
    //        printf("erro\n");
    //        break;
    //    case 0:
    //        close(pedidos[1]);
    //        close(serverProdutorGestorConsumidor[1]);
    //        close(serverConsumidorGestorProdutor[0]);
    //        int pid, status, x;
    //        PEDIDO pedido;
    //        GESTOR_PEDIDOS gp = createGestorPedidos(argv[1]);
    //        //codigo filho,  gere os pedidos, recebe pedidos novos do pai
    //        int retval1 = fcntl(pedidos[0], F_SETFL, fcntl(pedidos[0], F_GETFL) | O_NONBLOCK);
    //        int retval2 = fcntl(serverProdutorGestorConsumidor[0], F_SETFL, fcntl(serverProdutorGestorConsumidor[0], F_GETFL) | O_NONBLOCK);
    //        printf("Ret1 from fcntl: %d\n", retval1);
    //        printf("Ret2 from fcntl: %d\n", retval2);
    //        /*
    //        Este ciclo while serve para gerir os pedidos
    //        Ele a todo o momento verifica se algum pedido acabou (waitpid), e se de facto acabou ele atualiza
    //        */
    //        while(1) {
    //            while ((pid = waitpid(-1,&status,WNOHANG)) > 0) { //enquanto houverem pedidos terminados
    //                printf("Acabou um pedido\n");
    //                removerPedido(gp, pid); //remove do gestor de pedidos o pedido cujo processo acabou
    //            }
    //            while ((pedido = readPedido(pedidos[0])) != NULL) { //enquanto houver pedidos
    //                printf("Leu um pedido\n");
    //                setPedidoNth(pedido, i); i++;
    //                char* str = getPedidoStr(pedido);
    //                printf("pedido - %s\n", str);
    //                inserirPedido(gp, pedido);
    //                //printPedido(pedido);
    //            }
    //            if((read(serverProdutorGestorConsumidor[0], &x, sizeof(int))) > 0) { //se o processo pai quiser o estado do servidor, manda o estado do servidor
    //                //Se não houver nada para ler no pipe, este devolve -1 e continua execuçao, dado que o read neste pipe é nao bloqueador
    //                SERVER serverAtual = createServerFromGestor(gp); //cria o estado atual do servidor
    //                writeServer(serverAtual, serverConsumidorGestorProdutor[1]);
    //                //freeServer(&serverAtual);
    //            }
//
    //            usleep(50000); //espera 50 milissegundos
    //        }
    //        break;
    //    default:
    //        //codigo pai, apenas le input dos clientes, e manda os inputs para o processo que gere os pedidos
    //        close(pedidos[0]); //pai so vai escrever os pedidos para o gestor de pedidos
    //        close(serverProdutorGestorConsumidor[0]);
    //        close(serverConsumidorGestorProdutor[1]);
    //        while(1) {
    //            int c = -1;
    //            fd = open(myfifo, O_RDONLY);
    //            read(fd, &c, sizeof(int));
    //            //close(fd);
    //            if (c==0) { // 0 -> ler status, logo passamos a struct server
    //                write(serverProdutorGestorConsumidor[1], &c, sizeof(int)); //manda qualquer coisa para o gestor para ele saber que o processo pai precisa do estado do servidor
    //                SERVER server = readServer(serverConsumidorGestorProdutor[0]); //le o servidor no estado atual do pipe para onde o gestor escreveu
    //                close(fd);
    //                fd = open(myfifo, O_WRONLY);
    //                writeServer(server,fd); //manda o estado atual do servidor para o cliente, pelo fifo
    //            } else if (c==1) {
    //                /*
    //                Se c == 1, então é para ler um pedido, por isso logo asseguir le um pedido do fifo.
    //                Depois de ler o pedido, escreve o pedido para o processo que gere os pedidos (pelo fds[1]);
    //                */
    //                PEDIDO pedido = readPedido(fd);
    //                writePedido(pedido,pedidos[1]);
    //            }
    //            close(fd);
    //        }
    //        break;
    //    }

    /*
    CODIGO PARA TESTAR, mostra que funciona transmitir o estado do servidor por pipes
    ---------------------------------------------------------------------
    int fds[2];
    pipe(fds);
    if (fork() == 0) {
        //codigo filho, consumidor
        close(fds[1]);
        SERVER server1 = readServer(fds[0]);
        printf("[SERVER1]: \n");
        printServerStatus(server1);
    } else {
        //codigo pai, produtor
        close(fds[0]);
        SERVER server = createServer(argv[1]); //crio o server a partir do ficheiro de config
        writeServer(server, fds[1]); //escrevo o server para a pipe sem nome
        freeServer(&server); //dou free ao server
        close(fds[1]);
    }
    --------------------------------------------------------------------
    */
    
    return 1;
}