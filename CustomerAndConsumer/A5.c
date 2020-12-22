/******************************************
Auther:        Bochao Zhang
Banner ID:     B00748967
Date:          2020/11/29
Assignment:    CSCI3120 ASN5
 ******************************************/

//Library used
#include <pthread.h>
#include <stdlib.h>
#include <memory.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

//Buffer sized set to 5(max range 5)
#define BUFFER_SIZE 5
typedef int buffer_item;
buffer_item buffer[BUFFER_SIZE];
//Buffer contains three components
static int buffer_head, buffer_tail, buffer_full;

//Initialize the buffers
void buffer_init()
{   
    //Fill in the memory with buffer
    memset(buffer, 0, sizeof(buffer));
    buffer_head = buffer_tail = buffer_full = 0;
    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        buffer[i] = -1;
    }
}

//When a producer inserts item
void buffer_put(int pn, buffer_item val)
{
    printf("Producer %d inserted item %d into buffer[%d]\n", pn, val, buffer_head);
    buffer[buffer_head] = val;
    //Check if the buffer is full
    if (buffer_full)
    {
        buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    }
    //Renew buffer head and tail
    buffer_head = (buffer_head + 1) % BUFFER_SIZE;
    buffer_full = (buffer_head == buffer_tail);
}

//When a consumer removes a item
int buffer_remove(int pn, buffer_item *val)
{
    //Marker of item being removed
    int ret = 0;
    if (!(!buffer_full && buffer_head == buffer_tail))
    {
        *val = buffer[buffer_tail];
        buffer_full = 0;
        buffer[buffer_tail] = -1;
        printf("Consumer %d consumed item %d from buffer[%d]\n", pn, *val, buffer_tail);
        buffer_tail = (buffer_tail + 1) % BUFFER_SIZE;
    }
    ret = -1;
    return ret;
}

sem_t in, out;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Process of inserting an item
void insert_item(int pn, buffer_item val)
{
    pthread_mutex_lock(&mutex);
    buffer_put(pn, val);
    pthread_mutex_unlock(&mutex);
}

//Process of removing an item
void remove_item(int pn)
{
    pthread_mutex_lock(&mutex);
    buffer_item val;
    buffer_remove(pn, &val);
    pthread_mutex_unlock(&mutex);
}

//Create a prducer
void *producer(void *arg)
{
    buffer_item item;
    while (1)
    {
        int tm = rand() % 5; //Range from 0 to 4
        sleep(tm);
        sem_wait(&out);
        item = rand();
        insert_item(*(int *)arg, item);
        sem_post(&in);
    }
    return NULL;
}

//Create a consumer
void *consumer(void *arg)
{
    while (1)
    {
        int tm = rand() % 5; //Range from 0 to 4
        sleep(tm);
        sem_wait(&in);
        remove_item(*(int *)arg);
        sem_post(&out);
    }
}

//Main function
int main(int argc, char **argv)
{
    //Enable randomlization
    srand((unsigned)time(NULL));
    //Creat and reset all buffer to default
    buffer_init();
    if (sem_init(&in, 0, 0) == -1)
    {
        perror("sem init fail");
        exit(1);
    }
    if (sem_init(&out, 0, BUFFER_SIZE) == -1)
    {
        perror("sem init fail");
        exit(1);
    }
    //Read user's input from command line
    int t = atoi(argv[1]);
    int number_p = atoi(argv[2]), number_c = atoi(argv[3]);
    //Set sleeping length, # of producers, # of consumers
    pthread_t *producers = (pthread_t *)malloc(sizeof(pthread_t) * number_p);
    pthread_t *consumers = (pthread_t *)malloc(sizeof(pthread_t) * number_c);
    int *index_p = (int *)malloc(sizeof(int) * number_p);
    int *index_c = (int *)malloc(sizeof(int) * number_c);
    
    //Create thread of producers and consumers
    int i = 0;
    for (i = 0; i < number_p; i++)
    {
        index_p[i] = i;
        pthread_create(&producers[i], NULL, producer, (void *)&index_p[i]);
    }
    for (i = 0; i < number_c; i++)
    {
        index_c[i] = i;
        pthread_create(&consumers[i], NULL, consumer, (void *)&index_c[i]);
    }
    sleep(t);

    //Start the insert and consumer process
    for (i = 0; i < number_p; i++)
    {
        pthread_detach(producers[i]);
    }
    for (i = 0; i < number_c; i++)
    {
        pthread_detach(consumers[i]);
    }
    //Free memory
    free(producers), free(consumers);
    free(index_c), free(index_p);
    sem_destroy(&in), sem_destroy(&out);
    pthread_mutex_destroy(&mutex);
    return 0;
}
