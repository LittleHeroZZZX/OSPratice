/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-18 10:05:23
 * @LastEditTime: 2022-11-18 13:47:15
 * @Software: VSCode
 */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>

#define SHMKEY (key_t)20051336
#define SHMSIZE 1024


int main()
{
    sem_t *send_sem, *recv_sem;
    sem_unlink("send_sem");
    sem_unlink("recv_sem");
    send_sem = sem_open("send_sem", O_CREAT, 0666, 0);
    recv_sem = sem_open("recv_sem", O_CREAT, 0666, 0);
    if (fork() == 0)
    {
        int shmid;
        char *shmaddr;
        char buff[SHMSIZE];
        shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | 0666);
        if (shmid < 0)
        {
            printf("sender: shmget error\n");
            return 0;
        }
        shmaddr = shmat(shmid, 0, 0);
        if (shmaddr == (void *)-1)
        {
            printf("sender: shmat error\n");
            return 0;
        }
        printf("sender> ");
        scanf("%s", buff);
        strcpy(shmaddr, buff);
        sem_post(recv_sem);
        sem_wait(send_sem);
        strcpy(shmaddr, "over");
        shmdt(shmaddr);
        printf("sender: exit\n");
    }
    else
    {
        int shmid;
        char *shmaddr;
        char buff[SHMSIZE];
        shmid = shmget(SHMKEY, SHMSIZE, IPC_CREAT | 0666);
        if (shmid < 0)
        {
            printf("receiver: shmget error\n");
            return 0;
        }
        shmaddr = shmat(shmid, 0, 0);
        if (shmaddr == (void *)-1)
        {
            printf("receiver: shmat error\n");
            return 0;
        }
        sem_wait(recv_sem);
        strcpy(buff, shmaddr);
        printf("receiver: read \"%s\" from shared memory\n", buff);
        sem_post(send_sem);
        shmdt(shmaddr);
        printf("reveiver: exit\n");
        sem_close(send_sem);
        sem_close(recv_sem);
    }

}