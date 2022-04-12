//Programa cliente
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc==1) {
        //obter informação de utilização do programa cliente
        printf("./sdstore status\n");
        printf("./sdstore proc-file priority input-filename output-filename transformation-id-1 transformation-id-2 ...\n");
        return 1;
    } else if (argc==2 && strcmp(argv[1],"status") == 0) {
        //obter o estado de funcionamento do servidor.
        //dar print às tasks atuais, e a utilização das transformações
    }
    if (argc < 3) {
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