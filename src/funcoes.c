#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

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