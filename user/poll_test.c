#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include <time.h>

int main(){
    int fd = open ("/dev/msgqueue", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct pollfd pfd;
    pfd.fd =fd ;
    pfd.events =POLLIN;
    pfd.revents=0;
    time_t start = time(NULL);
    printf("strating poll...\n");
    fflush(stdout);

    int ret= poll(&pfd, 1, 5000);
    
    time_t end= time(NULL);
    printf("poll waited approximately %ld seconds\n", (long)(end-start));

    if(ret == 0){
        printf("Poll timed out: no events occured\n");
    }

    if(ret  < 0){
        perror("poll");
        close(fd);
        return 1;
    }
    printf("poll() returned : %d\n", ret);
    printf("revents: 0x%x\n", pfd.revents);

    if(pfd.revents & POLLIN){
        printf("Device is ready for reading\n");
    }
    if(pfd.revents & POLLOUT){
        printf("Device is ready for writing\n");
    }
    close(fd);
    return 0;
}