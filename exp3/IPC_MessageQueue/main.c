/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-11-17 20:42:18
 * @LastEditTime: 2022-11-18 14:32:52
 * @Software: VSCode
 */
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <fcntl.h>

#define MSGKEY (key_t)20051336
#define LEN 256
sem_t *queue_sem;

struct msgform
{
    long mtype;
    char mtext[LEN];
};
typedef struct msgform msgbuf;

msgbuf msg1, msg2;

void* sender1(void *arg)
{   
    int msgid;
    
    msgid = msgget(MSGKEY, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        printf("sender1: msgget error\n");
        return NULL;
    }
    msgbuf msg;
    msg.mtype = 1;
    char buff[LEN];
    while(1)
    {
        scanf("%s",buff);
        if (strcmp(buff, "exit") == 0)
            break;
        strcpy(msg.mtext, buff);
        sem_wait(queue_sem);
        int ret = msgsnd(msgid, &msg, LEN, 0);
        sem_post(queue_sem);
        if (ret < 0)
        {
            perror("sender1: msgsnd error");
        }      
    }
    strcpy(msg.mtext, "end1");
    sem_wait(queue_sem);
    msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
    sem_post(queue_sem);
    msgrcv(msgid, &msg, LEN, 2, 0);
    printf("sender1: %s\n", msg.mtext);
    
    printf("sender1: exit\n");
    // sleep(20);
    return NULL;

}

void* sender2(void *arg)
{
    // sem_wait(&send2_start);
    int msgid;
    msgid = msgget(MSGKEY, 0666);
    if (msgid < 0)
    {
        
        printf("sender2: msgget error\n");
        sem_post(queue_sem);
        return NULL;
    }
    msgbuf msg;
    msg.mtype = 1;
    char buff[1024];
    while(1)
    {
        scanf("%s",buff);
        if (strcmp(buff, "exit") == 0)
            break;
        strcpy(msg.mtext, buff);
        sem_wait(queue_sem);
        msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
        sem_post(queue_sem);
       
    }
    strcpy(msg.mtext, "end2");
    sem_wait(queue_sem);
    msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
    msgrcv(msgid, &msg, LEN, 3, 0);
    sem_post(queue_sem);
    printf("sender2: %s\n", msg.mtext);
    return NULL;
}

void* receiver(void *arg)
{
    int msgid;
    int cnt = 0;
    msgid = msgget(MSGKEY, IPC_CREAT | 0666);
    if (msgid < 0)
    {
        printf("receiver: msgget error\n");
        return NULL;
    }
    msgbuf msg;
    while(1)
    {
        sleep(1);
        int ret = msgrcv(msgid, &msg, LEN, 1, 0);
        printf("receiver: %s\n", msg.mtext);
        if (strcmp(msg.mtext, "end1") == 0)
        {
            msg.mtype = 2;
            strcpy(msg.mtext, "over1");
            msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
            cnt++;
        } else if (strcmp(msg.mtext, "end2") == 0)
        {

            msg.mtype = 3;
            strcpy(msg.mtext, "over2");
            msgsnd(msgid, &msg, strlen(msg.mtext) + 1, 0);
            cnt++;
        }
        
        if (cnt == 2)
            break;
        // sleep(1);
    }
    
    return NULL;
}

int main()
{



    pthread_t t1, t2, t3;
    queue_sem = sem_open("queue_sem", O_CREAT, 0666, 1);
    pthread_create(&t1, NULL, (void *)sender1, NULL);
    pthread_create(&t2, NULL, (void *)sender2, NULL);
    pthread_create(&t3, NULL, (void *)receiver, NULL);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    

    
    
    return 0;

}