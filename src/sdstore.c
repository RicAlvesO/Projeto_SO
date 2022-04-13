//Programa cliente
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "../libs/servidor.h"

int main(int argc, char* argv[]) {
    int fd, x;
    char* myfifo = "myfifo";
    if (argc==1) {
        //obter informação de utilização do programa cliente
        printf("./sdstore status\n");
        printf("./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n");
        return 1;
    } else if (argc==2 && strcmp(argv[1],"status") == 0) {
        //obter o estado de funcionamento do servidor.
        //dar print às tasks atuais, e a utilização das transformações
        mkfifo(myfifo,0666);
        fd = open(myfifo, O_WRONLY);
        x=0;
        write(fd,&x,sizeof(int));
        close(fd);
        //do outro lado do fifo, o servidor está a ouvir um inteiro
        //ao receber o numero 0, ele associa o numero 0 ao mostrar o status do servidor

        fd = open(myfifo, O_RDONLY);
        SERVER server = readServer(fd);
        printServerStatus(server); //queremos que o server dê print no terminal do cliente
        close(fd);
    } else if (argc < 3) {
        write(2,"NOT ENOUGH ARGUMENTS PROVIDED!\n",32);
        return -1;
    }

    if(access(argv[1], F_OK)==-1){
        write(2,"argv[1]: FILE NOT FOUND!\n",26);
    }

    for(int cur=3;cur<argc;cur++){
        //LOOP DE SEQUENCIAS DE TRANSFORMACOES A APLICAR
    }

    return 1;
}