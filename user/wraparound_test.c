#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(){
    int fd;
    char write_buffer[4090];
    char read_buffer[4090];
    char wrap_data[]= "ABCDEFGHIJKLMNOPQRST";

    memset(write_buffer, 'A', sizeof(write_buffer));
    memset(read_buffer, 0 , sizeof(read_buffer));

    fd= open("/dev/msgqueue", O_RDWR);
    if(fd==-1){
        perror("open failed");
        return EXIT_FAILURE;
    }

    ssize_t bytes_to_write;
    bytes_to_write= write(fd, write_buffer, sizeof(write_buffer));
    if(bytes_to_write == 0){
        perror("no bytes written");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_to_write == -1){
        perror("failed to write data");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_to_write < sizeof(write_buffer)){
        printf("failed to write data, it wrote, %zd", bytes_to_write);
        close(fd);
        return EXIT_FAILURE;
    }

    printf("number of bytes it wrote : %zd\n", bytes_to_write );

    ssize_t bytes_to_read;
    bytes_to_read= read(fd, read_buffer, sizeof(read_buffer));
    if(bytes_to_read == -1){
        perror("read failed");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_to_read ==0){
        perror("no bytes to read");
        close(fd);
        return EXIT_FAILURE;
    }
    if(bytes_to_read < sizeof(read_buffer)){
        printf("reading failed in bteween it just read :%zd\n", bytes_to_read);
        close(fd);
        return EXIT_FAILURE;
    }
    printf("number of bytes to read is :%zd\n", bytes_to_read);

    int  res;
    res=memcmp(read_buffer, write_buffer, sizeof(read_buffer));
    if(res ==0){
        printf("data verfication is successful\n");
    }
    if(res < 0){
        printf("data verification failed\n");
        close(fd);
        return EXIT_FAILURE;
    }
    if(res > 0){
        printf("mismatch in data\n");
        close(fd);
        return EXIT_FAILURE;
    }

    ssize_t wrap_bytes_written;
    wrap_bytes_written= write(fd, wrap_data, 20);

    if(wrap_bytes_written != 20){
        printf("Expected 20 bytes, but wrote %zd\n", wrap_bytes_written);
        close(fd);
        return EXIT_FAILURE;
    }
    printf("Wraparound write successful: %zd bytes\n", wrap_bytes_written);

    memset(read_buffer, 0, sizeof(read_buffer));
    ssize_t wrap_bytes_read;
    wrap_bytes_read = read(fd, read_buffer, 20);


    if(wrap_bytes_read == -1){
        perror("read error");
        close(fd);
        return EXIT_FAILURE;
    }
    if(wrap_bytes_read == 0){
        printf("nothing was read");
        close(fd);
        return EXIT_FAILURE;
    }
    if(wrap_bytes_read != 20){
        printf("data is partially read");
        close(fd);
        return EXIT_FAILURE;
    }
    if(wrap_bytes_read == 20){
        printf("Wraparound read successful: %zd bytes\n", wrap_bytes_read);
    }
    
    int first_check;

    first_check= memcmp(read_buffer, wrap_data, 20);

    if(first_check == 0 ){
        printf("wraparound data verification successful\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    printf("wraparound data verification failed\n");
    close(fd);
    return EXIT_FAILURE;
}