#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include "../msgqueue_ioctl.h"

int main(){
    int fd;
    fd= open("/dev/msgqueue", O_RDWR);
    if(fd < 0){
        perror("failed to open");
        return EXIT_FAILURE;
    }
    if (ioctl(fd, MSGQUEUE_CLEAR) < 0) {
        perror("queue clear failed");
        close(fd);
        return EXIT_FAILURE;
    }
    char buffer='A';
    ssize_t bytes_written= write(fd, &buffer, sizeof(buffer));
    if (bytes_written != sizeof(buffer)) {
        fprintf(stderr, "Expected to write 1 byte, got %zd\n", bytes_written);
        close(fd);
        return EXIT_FAILURE;
    }

    size_t count1=0;
    size_t count2=0;
    int ret = ioctl(fd, MSGQUEUE_GET_COUNT,&count1);

    if (ret < 0) {
        perror("first ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }
    int ret2 = ioctl(fd, MSGQUEUE_GET_COUNT,&count2);
    if (ret2 < 0) {
        perror("second ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }

    char read_buffer;
    ssize_t bytes_read= read(fd, &read_buffer, sizeof(read_buffer));
    if(bytes_read < 0){
        perror("read failed");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_read != sizeof(read_buffer)){
        fprintf(stderr, "Expected to read 1 byte, got %zd\n", bytes_read);
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Read character: %c\n", read_buffer);

    size_t count3=0;
    int ret3= ioctl(fd, MSGQUEUE_GET_COUNT, &count3);
    if (ret3 < 0) {
        perror("third ioctl failed");
        close(fd);
        return EXIT_FAILURE;
    }
    int invalid_ret = ioctl(fd, _IO(MSGQUEUE_MAGIC, 99));

    if (invalid_ret != -1) {
        fprintf(stderr, "Invalid ioctl unexpectedly succeeded\n");
        close(fd);
        return EXIT_FAILURE;
    }

    if (errno != ENOTTY) {
        perror("Unexpected ioctl error");
        close(fd);
        return EXIT_FAILURE;
    }

printf("Invalid ioctl correctly returned ENOTTY\n");

    printf("Bytes written: %zd\n", bytes_written);
    printf("Total count of first ioctl call: %zu\n", count1);
    printf("Total count of second ioctl call: %zu\n", count2);
    printf("Total count of third ioctl call: %zu\n", count3);

    close(fd);
    return EXIT_SUCCESS;


}