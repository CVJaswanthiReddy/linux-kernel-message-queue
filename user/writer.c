#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(){

    int fd;
    fd= open("/dev/msgqueue", O_WRONLY);
    if(fd ==-1){
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    const char *message = "hello from user space!";
    size_t message_length =strlen(message);
    for(int i=0; i< 100; i++){
        size_t total_written=0;
        ssize_t bytes_written;
        while(total_written < message_length){

        
        bytes_written=write(fd, message + total_written, message_length - total_written);

        if(bytes_written == -1){
            perror("write failed");
            close(fd);
            return EXIT_FAILURE;
        }
        if(bytes_written == 0){
            fprintf(stderr, "write returned 0; no progress made\n");            
            close(fd);
            return EXIT_FAILURE;
        }
        printf("Bytes written: %zd\n", bytes_written);
        total_written += bytes_written;
    }
    }

    
    close(fd);
}