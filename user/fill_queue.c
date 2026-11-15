#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
    int fd = open("/dev/msgqueue", O_WRONLY);
    if(fd < 0){
        perror("open");
        return 1;
    }

    char buffer[4096];
    memset(buffer, 'A', sizeof(buffer));

    size_t total_written=0;
    while(total_written < sizeof(buffer)){
        ssize_t bytes_written= write(fd, buffer+total_written, sizeof(buffer) - total_written);
        if(bytes_written < 0){
            perror("write");
            close(fd);
            return 1;
        }
        if(bytes_written == 0){
            fprintf(stderr, "Write made no progress\n");
            close(fd);
            return 1;
        }
        total_written+= bytes_written;
    }
    printf("Total bytes written: %zu\n", total_written);
    
    close(fd);
    return 0;
}

