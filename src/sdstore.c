//Programa cliente
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include "../libs/servidor.h"

// Function that handles client requests
int requests(int argc, char* argv[]){

    // Check for right command
    if (strcmp(argv[1], "proc-file") != 0){
        write(2, "COMMAND NOT FOUND!\n", 20);
        return -1;
    }

    // First argument to check + priority
    int cur=2, prio=0;

    // Check if priority is given
    if(isdigit(atoi(argv[1]))){
        cur++;
        prio==atoi(argv[1]);
    }

    // Get input file + verify existence
    char *in=strdup(argv[cur]);
    cur++;
    if (access(in, F_OK) == -1){
        write(2, "INPUT FILE NOT FOUND!\n", 23);
        return -1;
    }

    // Get where to output
    char *out=strdup(argv[cur]);
    cur++;

    // Loop for the remaining operations to apply
    while(cur < argc)
    {
        cur++;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    int fd, x;
    char* myfifo = "myfifo";

    if (argc==1) {
    
        // A basic help menu for the client
        write(1,"./sdstore status\n#Gives information about the current server status.\n",70);
        write(1,"./sdstore proc-file <priority> input-filename output-filename transformation-id-1 transformation-id-2 ...\n",107);
        write(1,"#Submit file prossessing and storage requests acording to the command given.\nThe <priority> flag is an integer and CAN be used to boost the priority of a given command.\n",170);
        return 1;
    
    }else if (argc==2 && strcmp(argv[1],"status") == 0) {
        
        // Get server status, show the current tasks and transfs
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
        return 1;

    }else if (argc < 3){

        // Not enough arguments for requests
        write(2,"NOT ENOUGH ARGUMENTS PROVIDED!\n",32);
        return -1;
    }

    return requests(argc, argv);
}