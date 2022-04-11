//Programa servidor
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc!=3) {
        printf("numero de argumentos incorreto\n");
        return -1;
    }
    //argv[1] -> caminho para ficheiro de configuração
    //argv[2] -> caminho para a pasta onde os executáveis das transformações estão guardados
    return 1;
}