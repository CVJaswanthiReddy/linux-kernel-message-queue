#include <stdio.h> // standard input/output
#include <fcntl.h> //it is for file control
#include <unistd.h>  //it is the toolbox used to interact with the os core(kernel)
#include <stdlib.h> //standarad library

int main(){
    int fd;
    fd= open("/dev/msgqueue", O_RDWR);
    if(fd==-1){
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    printf("Device opened successfully\n");
    close(fd);
    
    printf("closing\n");
    return 0;
    
    

}