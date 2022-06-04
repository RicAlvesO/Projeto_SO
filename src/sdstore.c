//Programa cliente
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../libs/servidor.h"
#include "../libs/pedido.h"
#include "../libs/funcoes.h"
#include "../libs/pacote.h"

int main(int argc, char* argv[]) {
    int fdProdutor, fdConsumidor, i=0;
    char* fifo_geral = "fifos/fifo_geral";
    pid_t pid = getpid();

    char* fifoProdutor = getPrivateFifoPath(pid,1);
    char* fifoConsumidor = getPrivateFifoPath(pid,0);
    //return 0;

    mkfifo(fifo_geral,0666);
    mkfifo(fifoConsumidor,0666);
    mkfifo(fifoProdutor,0666);

    if (argc==1) {
    
        // A basic help menu for the client
        write(1,"./sdstore status\n#Gives information about the current server status.\n",70);
        write(1,"./sdstore proc-file <priority> input-filename output-filename transformation-id-1 transformation-id-2 ...\n",107);
        write(1,"#Submit file prossessing and storage requests acording to the command given.\nThe <priority> flag is an integer and CAN be used to boost the priority of a given command.\n",170);
        return 1;
    
    }else if (argc==2 && strcmp(argv[1],"status") == 0) {
        
        // Get server status, show the current tasks and transfs

        /*
        Depois de ter aberto o fifo geral, manda para la a string contendo o nome do fifo privado deste cliente,
        para que a informacao entre o servidor e este cliente nao seja "roubada" por outros clientes.
        */
        if ((fdProdutor = open(fifo_geral, O_WRONLY | O_NDELAY)) < 0) {
            write(2, "O servidor nao aceita pedidos de momento\n", 42);
            return 1;
        }
        PACOTE pacote = createServerStatusPacote(pid);
        writePacote(pacote, fdProdutor);
        freePacote(&pacote);
        close(fdProdutor);
        fdConsumidor = open(fifoConsumidor, O_RDONLY);

        SERVER server = readServer(fdConsumidor);
        printServerStatus(server); //queremos que o server dê print no terminal do cliente
        close(fdConsumidor);
        return 1;

    } else if (argc > 2 && strcmp(argv[1],"proc-file") == 0) {
        //Apply transformations

        /*
        Depois de ter aberto o fifo geral, manda para la a string contendo o nome do fifo privado deste cliente,
        para que a informacao entre o servidor e este cliente nao seja "roubada" por outros clientes.
        */
        if ((fdProdutor = open(fifo_geral, O_WRONLY | O_NDELAY)) < 0) {
            write(2, "O servidor nao aceita pedidos de momento\n", 42);
            return 1;
        }
        PACOTE pacote = createSendingPedidoPacote(pid);
        writePacote(pacote, fdProdutor);
        //write(fdProdutor, &pid, sizeof(pid_t));
        close(fdProdutor);
        fdProdutor = open(fifoProdutor, O_WRONLY);
        //int x = 1;
        //write(fdProdutor, &x, sizeof(int));

        int cur=2,priority=1;
        if(strcmp(argv[cur],"-p")==0){
            cur++;
            priority=atoi(argv[cur]);
            cur++;
        }

        char* inputPath = argv[cur];
        cur++;
        char* outputPath = argv[cur];
        cur++;
        int ntransf = argc - cur;
        char* transformacoes[ntransf];
        for (int i=0; i<ntransf; i++) {
            transformacoes[i] = argv[i + cur];
        }
        PEDIDO pedido = createPedido(priority, inputPath, outputPath, ntransf, transformacoes); i++;
        writePedido(pedido, fdProdutor);
        close(fdProdutor);

        char buffer[128];
        fdConsumidor = open(fifoConsumidor, O_RDONLY);
        while (readln(fdConsumidor, buffer, 128) > 0) {
            /*
            Enquanto o servidor nao fecha o fifo, o cliente recebe mensagens do tipo 'pending', 'processing', etc,
            e da print para o ecra do cliente.
            Se recebe a mensagem end, é suposto parar de ler input
            */
            if (strcmp(buffer, "end\n") == 0) {
                close(fdConsumidor);
                close(fdProdutor);
                return 2;
            }
            write(1,buffer,strlen(buffer));
            fflush(stdout);
        }

        return 2;

    } else if (argc < 3){

        // Not enough arguments for requests
        write(2,"[ERROR]: Argumentos insuficientes\n",35);
        return -1;
    }

    return 1;
}