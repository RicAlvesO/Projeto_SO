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

// Function that handles client requests
int requests(int argc, char* argv[]){

    // Check for right command
    if (strcmp(argv[1], "proc-file") != 0){
        write(2, "COMMAND NOT FOUND!\n", 20);
        return -1;
    }

    // First argument to check + priority
    int cur=2, prio=0;

    // Check if priority is given
    if(isdigit(atoi(argv[1]))){
        cur++;
        prio=atoi(argv[1]);
    }

    // Get input file + verify existence
    char *in=strdup(argv[cur]);
    cur++;
    if (access(in, F_OK) == -1){
        write(2, "INPUT FILE NOT FOUND!\n", 23);
        return -1;
    }

    // Get where to output
    //char *out=strdup(argv[cur]);
    cur++;

    // Loop for the remaining operations to apply
    while(cur < argc)
    {
        cur++;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    int fdProdutor, fdConsumidor, x, i=0;
    char myfifo[128], fifoProdutor[128], fifoConsumidor[128];
    myfifo[0] = '\0';
    char* fifo_geral = "fifos/fifo_geral";
    char pidStr[20];
    int pid = getpid();
    sprintf(pidStr,"%d", pid);
    strcat(myfifo, "fifos/fifo");
    strcat(myfifo, pidStr);
    strcpy(fifoProdutor, myfifo);
    strcpy(fifoConsumidor, myfifo);
    strcat(fifoProdutor, "-cliente_produtor");
    strcat(fifoConsumidor, "-cliente_consumidor");
    //return 0;

    if (argc==1) {
    
        // A basic help menu for the client
        write(1,"./sdstore status\n#Gives information about the current server status.\n",70);
        write(1,"./sdstore proc-file <priority> input-filename output-filename transformation-id-1 transformation-id-2 ...\n",107);
        write(1,"#Submit file prossessing and storage requests acording to the command given.\nThe <priority> flag is an integer and CAN be used to boost the priority of a given command.\n",170);
        return 1;
    
    }else if (argc==2 && strcmp(argv[1],"status") == 0) {
        
        // Get server status, show the current tasks and transfs
        if (mkfifo(fifo_geral,0666) == -1) {
            write(1,"[CLIENTE]: fifo_geral ja criado\n", 33);
        }
        if (mkfifo(fifoConsumidor,0666) == -1) {
            write(1,"[CLIENTE]: fifo consumidor ja criado\n", 38);
        }
        if (mkfifo(fifoProdutor,0666) == -1) {
            write(1,"[CLIENTE]: fifo produtor ja criado\n", 36);
        }

        /*
        Depois de ter aberto o fifo geral, manda para la a string contendo o nome do fifo privado deste cliente,
        para que a informacao entre o servidor e este cliente nao seja "roubada" por outros clientes.
        */
        if ((fdProdutor = open(fifo_geral, O_WRONLY)) == -1) {
            write(1,"[CLIENTE]: nao conseguiu abrir fifo_geral\n",43);
        }
        write (fdProdutor, myfifo, sizeof(myfifo)); //maximo 128 caracteres
        close(fdProdutor);
        fdProdutor = open(fifoProdutor, O_WRONLY);
        fdConsumidor = open(fifoConsumidor, O_RDONLY);
        x=0;
        write(fdProdutor,&x,sizeof(int)); //avisa o servidor que precisa do estado do servidor
        close(fdProdutor);
        //do outro lado do fifo, o servidor está a ouvir um inteiro
        //ao receber o numero 0, ele associa o numero 0 ao mostrar o status do servidor

        SERVER server = readServer(fdConsumidor);
        printServerStatus(server); //queremos que o server dê print no terminal do cliente
        close(fdConsumidor);
        return 1;

    } else if (argc > 2 && strcmp(argv[1],"proc-file") == 0) {
        //Apply transformations

        if (mkfifo(fifo_geral,0666) == -1) {
            write(1,"[CLIENTE]: fifo_geral ja criado\n", 33);
        }
        if (mkfifo(fifoConsumidor,0666) == -1) {
            write(1,"[CLIENTE]: fifo consumidor ja criado\n", 38);
        }
        if (mkfifo(fifoProdutor,0666) == -1) {
            write(1,"[CLIENTE]: fifo produtor ja criado\n", 36);
        }

        /*
        Depois de ter aberto o fifo geral, manda para la a string contendo o nome do fifo privado deste cliente,
        para que a informacao entre o servidor e este cliente nao seja "roubada" por outros clientes.
        */
        if ((fdProdutor = open(fifo_geral, O_WRONLY)) == -1) {
            write(1,"[CLIENTE]: nao conseguiu abrir fifo_geral\n",43);
        }

        write (fdProdutor, myfifo, sizeof(myfifo)); //maximo 128 caracteres
        close(fdProdutor);
        fdProdutor = open(fifoProdutor, O_WRONLY);
        fdConsumidor = open(fifoConsumidor, O_RDONLY);

        int priority = atoi(argv[2]);
        char* inputPath = argv[3];
        char* outputPath = argv[4];
        int ntransf = argc - 5;
        char* transformacoes[ntransf];
        for (int i=0; i<ntransf; i++) {
            transformacoes[i] = argv[i + 5];
        }
        PEDIDO pedido = createPedido(priority, inputPath, outputPath, ntransf, transformacoes); i++;

        //fdProdutor = open(fifo_geral, O_WRONLY);
        x=1;
        write(fdProdutor,&x,sizeof(int)); //escreve um 1 para o servidor, indicando que vem ai um pedido
        writePedido(pedido,fdProdutor); //escreve o pedido, estando o servidor pronto a recebe-lo.
        close(fdProdutor);

        char buffer[128];
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
        write(2,"NOT ENOUGH ARGUMENTS PROVIDED!\n",32);
        return -1;
    }

    return requests(argc, argv);
}