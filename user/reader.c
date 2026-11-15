#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd;
    fd=open("/dev/msgqueue", O_RDONLY);
    
    if(fd ==-1){
        perror("open failed");
        exit(EXIT_FAILURE);
    }
    ssize_t total_read = 0;
    while( total_read < 6600){
        ssize_t bytes_read;

        char buffer[100];

        bytes_read= read(fd,buffer, sizeof(buffer)-1 );

        if(bytes_read ==-1){
            perror("read failed");
            close(fd);
            return EXIT_FAILURE;
        }
        if(bytes_read == 0){
            break;
        }
    
        printf("Bytes read: %zd\n", bytes_read);
        total_read += bytes_read;
        buffer[bytes_read]='\0';
        printf("received message : %s\n", buffer);
    }
    

    
    close(fd);

    return EXIT_SUCCESS;
}