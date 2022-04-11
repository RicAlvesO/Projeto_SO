//Programa cliente
#include <stdio.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("numero de argumentos unsuficiente\n");
        return -1;
    }
    //argv[1] -> caminho do ficheiro a ser processado
    //argv[2] -> caminho onde se guarda a nova versão do ficheiro apos transformaçoes
    //argv[3] ... argv[x] -> sequencia de identificadores de transformações a aplicar
    return 1;
}