#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(){
    int fd;
    fd= open("/dev/msgqueue", O_WRONLY | O_NONBLOCK);

    if(fd < 0){
        perror("open failed");
        return EXIT_FAILURE;
    }
    char buffer ='C';
    ssize_t bytes_written;
    bytes_written= write(fd, &buffer, sizeof(buffer));

    if(bytes_written == -1 && errno ==EAGAIN){
        printf("Queue is full. Nonblocking write returned EAGAIN.\n");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_written == -1){
        perror("writed failed for some reason");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("number of bytes writtern: %zd\n", bytes_written);

    close(fd);
    return EXIT_SUCCESS;
}
