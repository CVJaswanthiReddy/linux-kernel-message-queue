#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "../msgqueue_ioctl.h"

int main(){
    int fd;
    fd= open("/dev/msgqueue", O_RDWR);

    if(fd < 0){
        perror("open failed");
        return EXIT_FAILURE;
    }

    char const *message= "HELLO";
    ssize_t bytes_written;
    size_t message_length = strlen(message);
    bytes_written= write(fd, message, message_length);

    if(bytes_written < 0){
        perror("writing failed");
        close(fd);
        return EXIT_FAILURE;
    }

    if(bytes_written != (ssize_t)(message_length)){
        printf("partially written");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Total number of bytes_written :%zd\n", bytes_written);
    size_t count1=0;
    int res= ioctl(fd, MSGQUEUE_GET_COUNT, &count1);
    if(res < 0){
        perror("ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("ioctl count before is : %zu\n", count1);

    int res_clear= ioctl(fd, MSGQUEUE_CLEAR);
    if (res_clear < 0) {
        perror("clear ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }

    size_t count2=0;
    int res2= ioctl(fd, MSGQUEUE_GET_COUNT, &count2);
    if(res2 < 0){
        perror("ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("ioctl count after is : %zu\n", count2);
    close(fd);
    return EXIT_SUCCESS;

}