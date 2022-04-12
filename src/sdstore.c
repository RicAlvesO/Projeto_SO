//Programa cliente
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
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