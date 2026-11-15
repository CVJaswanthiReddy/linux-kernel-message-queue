#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int fd;
    fd= open("/dev/msgqueue", O_WRONLY);

    if(fd < 0){
        perror("open failed");
        return EXIT_FAILURE;
    }

    char buffer = 'B';
    ssize_t bytes_written;
    bytes_written = write( fd, &buffer, sizeof(buffer));

    if(bytes_written == -1){
        perror("failed in writing data");
        close(fd);
        return EXIT_FAILURE;
    }

    printf("number of bytes written : %zd\n", bytes_written);
    close(fd);
    return EXIT_SUCCESS;
}
