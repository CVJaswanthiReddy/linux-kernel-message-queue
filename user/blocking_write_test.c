#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
    char buffer[4096];
    int fd;
    memset(buffer, 'A', sizeof(buffer));

    fd = open("/dev/msgqueue", O_WRONLY);

    if(fd < 0){
        perror("open failed \n");
        return EXIT_FAILURE;
    }

    ssize_t bytes_written;
    bytes_written =write(fd, buffer, sizeof(buffer));
    if(bytes_written < 0){
        perror("error in writting\n");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_written == sizeof(buffer)){
        printf("data written succesfully\n");
        close(fd);
        return EXIT_SUCCESS;
    }
   
    if(bytes_written < sizeof(buffer)){
        printf("Bytes written: %zd\n", bytes_written);
    }

    close(fd);

}