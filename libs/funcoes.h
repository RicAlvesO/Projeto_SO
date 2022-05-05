#include <stddef.h>
#include <sys/types.h>

int readChar(int fd, char* c);
ssize_t readln(int fd, char* line, size_t size);
int contarBytes(char* path);