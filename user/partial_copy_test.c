#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int main(){
    long page_size = sysconf(_SC_PAGESIZE);
    printf("page size : %ld bytes \n", page_size);

    size_t total_size = 2 * page_size;
    void *memory;

    memory = mmap(NULL,
                  total_size,
                  PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS,
                  -1,
                   0   
                );
    
    if(memory == MAP_FAILED){
        perror("mmap failed");
        return 1;
    }
    printf("Memory allocated successfully\n");

    char *buffer = (char *) memory;
    char *second_page = buffer + page_size;

    printf("First page address : %p\n", (void *)buffer);
    printf("Second page address: %p\n" , (void *)second_page);

    
    if(mprotect(second_page, page_size, PROT_NONE) == -1){
        perror("mprotect failed");
        munmap(memory, total_size);
        return 1;
    }
    printf("second page protection applied successfully\n ");

    char *test_buffer = buffer + page_size -8;
    memcpy(test_buffer, "ABCDEFGH", 8);
    printf("Test buffer address: %p\n", (void *)test_buffer);

    int fd= open("/dev/msgqueue", O_RDWR);
    if(fd < 0){
        perror("open failed");
        munmap(memory, total_size);
        return 1;
    }
    printf("Device opened successfully");

    ssize_t bytes_written = write(fd, test_buffer, 16);
    printf("Byes requested: 16\n");
    printf("Bytes actually written: %zd\n", bytes_written);


    ssize_t bytes_read = read(fd, test_buffer, 16);

    printf("bytes requested for reading: 16\n");
    printf("bytes actually read: %zd\n", bytes_read);

    close(fd);
    munmap(memory, total_size);
}