/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-11 13:46:35
 * @LastEditTime: 2022-11-18 13:20:11
 * @Software: VSCode
 */
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PAGE_SIZE 4096
#define BUFFER_SIZE 16 * PAGE_SIZE

int testPipeBufferSize(int pipefd) {
    int size = 0;
    while (1) {
        if (write(pipefd, "a", 1) == -1) {
            break;
        }
        size++;
    }
    return size;
}

int clearPipe(int fd) {
    int size = 0;
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    char buf[1];
    while (read(fd, buf, 1) > 0) size++;
    fcntl(fd, F_SETFL, flags);
    return size;
}

int main() {
    int fd[2];
    if (pipe(fd) < 0) {
        perror("pipe create error");
        return -1;
    }
    sem_t *write1_mutex, *write2_mutex, *read_mutex;
    write1_mutex = sem_open("write1_mutex", O_CREAT, 0666, 0);
    write2_mutex = sem_open("write2_mutex", O_CREAT, 0666, 0);
    read_mutex = sem_open("read_mutex", O_CREAT, 0666, 0);

    if (fork() == 0) {
        printf(
            "Child 1:\tEnter child1 process, which is used to test pipe buffer "
            "size\n");
        int flags = fcntl(fd[1], F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(fd[1], F_SETFL, flags);
        int size = testPipeBufferSize(fd[1]);
        flags &= ~O_NONBLOCK;
        fcntl(fd[1], F_SETFL, flags);
        printf("Child 1:\tPipe buffer size is %d bytes\n", size);
        printf("Child 1:\tQuit child1 process\n");
        exit(0);
    } else if (fork() == 0) {
        printf(
            "Child 2:\tEnter child2 process, which is used to print few text to "
            "pipe\n");
        sem_wait(write1_mutex);
        int n = 0;
        n = write(fd[1], "Hello World", 11);
        printf("Child 2:\tWrite %d bytes to pipe\n", n);
        sem_post(read_mutex);
        printf("Child 2:\tQuit child2 process\n");
        exit(0);
    } else if (fork() == 0) {
        printf(
            "Child 3:\tEnter child3 process, which is used to test blocked "
            "write "
            "to pipe\n");
        sem_wait(write2_mutex);
        int n = 0;
        printf("Child 3:\tStart to write to pipe\n");
        for (int i = 0; i < BUFFER_SIZE + PAGE_SIZE; i++) {
            write(fd[1], "a", 1);
            n++;
        }
        printf("Child 3:\tWrite %d bytes to pipe\n", n);
        printf("Child 3:\tQuit child3 process\n");
        exit(0);
    } else {
        printf(
            "Parent:   \tEnter parent process, which is used to read from pipe\n");
        wait(NULL);
        int n = clearPipe(fd[0]);
        printf("Parent:  \tClear %d bytes from pipe\n", n);
        sem_post(write1_mutex);
        sem_wait(read_mutex);
        n = 0;
        fcntl(fd[0], F_SETFL, O_NONBLOCK);
        n = clearPipe(fd[0]);
        printf("Parent:   \tRead %d bytes from pipe\n", n);
        sem_post(write2_mutex);
        sleep(1);
        for (int i = 0; i < 3; i++)
        {
            printf("...\n");
            sleep(1);
        }
        printf("Parent:  \tstart to clear pipe\n");
        n = clearPipe(fd[0]);
        printf("Parent:  \tClear %d bytes from pipe\n", n);

        close(fd[0]);
        close(fd[1]);

        sem_close(write1_mutex);
        sem_close(write2_mutex);
        
        exit(0);
    }
}
