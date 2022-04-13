// Programa servidor
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include "../libs/servidor.h"

// Verify If Bin Files Exists
int verifyBinFiles(char* path){
    int flag = 1;
    char *bins[]={"/bcompress","/bdecompress","/decrypt","/encrypt","/gcompress","/gdecompress","/nop"};
    for(int i=0; i<7; i++){
    
        // Create file destinations
        char str[(sizeof(bins[i]) + sizeof(path)) / sizeof(char)] = "";
        if(path[strlen(path)-1]=="/")path[strlen(path)-1]="\0";
        strcat(str, path);
        strcat(str, bins[i]);
        
        // Verify file existance
        if (access(str, F_OK) == -1){
            
            // Make ERROR message
            char error[(sizeof(bins[i])/8) + 12] = "";
            strcat(error, bins[i]);
            strcat(error, "NOT FOUND!\n");
            
            // Print ERROR message
            write(2, error, strlen(error));
            flag=0;
        }
    }
    return flag;
}

// Main Function
int main(int argc, char* argv[]) {
    char* myfifo = "myfifo";
    int fd;
    
    // Argument Verifications
    if (argc != 3){
        write(2, "NOT ENOUGH ARGUMENTS PROVIDED!\n", 32);
        return -1;
    }
    // Config File Verification
    if (access(argv[1], F_OK) == -1){
        write(2, "CONFIG FILE NOT FOUND!\n", 24);
        return -1;
    }
    //Bin Files Verification
    /*
    * Verifies the folder in the argument 
    * To work bins should be generated prior to using this program
    * To generate run `make` on libs folder
    */
    if(!verifyBinFiles(argv[2]))return -1;

    SERVER server = createServer(argv[1]); //crio o server a partir do ficheiro de config
    //printServerStatus(server);

    mkfifo(myfifo,0666);

    while(1) {
        int c = -1;
        fd = open(myfifo, O_RDONLY);
        read(fd, &c, sizeof(int));
        close(fd);
        if (c==0) { // 0 -> ler status, logo passamos a struct server
            fd = open(myfifo, O_WRONLY);
            writeServer(server,fd);
            close(fd);
        }

    }

    /*
    CODIGO PARA TESTAR, mostra que funciona transmitir o estado do servidor por pipes
    ---------------------------------------------------------------------
    int fds[2];
    pipe(fds);
    if (fork() == 0) {
        //codigo filho, consumidor
        close(fds[1]);
        SERVER server1 = readServer(fds[0]);
        printf("[SERVER1]: \n");
        printServerStatus(server1);
    } else {
        //codigo pai, produtor
        close(fds[0]);
        SERVER server = createServer(argv[1]); //crio o server a partir do ficheiro de config
        writeServer(server, fds[1]); //escrevo o server para a pipe sem nome
        freeServer(&server); //dou free ao server
        close(fds[1]);
    }
    --------------------------------------------------------------------
    */

    return 1;
}