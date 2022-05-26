#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

// Reads char from certain file descriptor
int readChar(int fd, char* c) {
    int ret = read(fd, c, 1);

    return ret;
}

// Reads line from certain file descriptor
ssize_t readln(int fd, char* line, size_t size) {
    char c;
    int i=0;
    while (i<size && c!='\n') {
        if (!readChar(fd, &c)) { //eof, nao leu nada
            if (i==0) return 0; //linha vazia
            else c='\n';
        }
        line[i] = c;
        i++;
    }
    if (i!=size) line[i] = '\0';
    return i; //tamanho da linha, incluindo o \n
}

/*
Recebe o caminho para um ficheiro que se assume que exista, e conta as bytes que existem no ficheiro
*/
int contarBytes(char* path) {
    int fds[2], status;
    char bytesRecebidas[20];
    pipe(fds);

    switch(fork()) {
        case -1:
            perror("Fork não foi efetuado");
            return -1;
        case 0:
            close(fds[0]);
            //Assume-se que o ficheiro existe.
            int fileParaLer = open(path, O_RDONLY, 0666);
            dup2(fileParaLer, 0); //em vez de ler do stdin, le do ficheiro que queremos saber o numero de bytes
            close(fileParaLer);

            dup2(fds[1], 1);
            close(fds[1]);

            execlp("wc", "wc", "-c", NULL); //conta o numero de bytes do ficheiro fileParaLer, e coloca no pipe

            _exit(0);
            break;
        default:
            close(fds[1]);

            read(fds[0], bytesRecebidas, 20); //coloca no array bytesRecebidas o numero de bytes que le do pipe
            
            break;
    }

    wait(&status); //espera pelo filho
    return atoi(bytesRecebidas);
}