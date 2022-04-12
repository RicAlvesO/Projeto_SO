// Programa servidor
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    if (argc != 3){
        write(2, "NOT ENOUGH ARGUMENTS PROVIDED!\n", 32);
        return -1;
    }

    if (access(argv[1], F_OK) == -1)
    {
        write(2, "argv[1]: FILE NOT FOUND!\n", 26);
    }

    //argv[1] -> caminho para ficheiro de configuração
    //argv[2] -> caminho para a pasta onde os executáveis das transformações estão guardados
    return 1;
}