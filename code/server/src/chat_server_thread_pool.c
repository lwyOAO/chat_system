#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "chat_server_thread_pool.h"

thread_pool_t pool;
extern int client_count;
extern client_t clients[MAX_CLIENTS];

void *worker_thread(void *arg)
{
    while (1)
    {
        task_t *task;

        // Lock the mutex to access the task queue
        pthread_mutex_lock(&pool.mutex);

        // Wait for tasks if queue is empty
        while (pool.task_count == 0)
        {
            pthread_cond_wait(&pool.cond, &pool.mutex);
        }

        // Retrieve task from the queue
        task = pool.tasks[--pool.task_count];

        // Unlock the mutex after retrieving the task
        pthread_mutex_unlock(&pool.mutex);

        // 对头部反序列化

        // Process the client's message
        printf("Thread handling message from client %d: %s\n", task->sockfd, task->message);

        // Reply to the client (optional)
        for(int i = 0; i < client_count; i++)
        {
            if(clients[i].sockfd != task->sockfd)
            {
                send(clients[i].sockfd, task->message, strlen(task->message), 0);
            }
        }

        // Free the task memory
        free(task);
    }

    return NULL;
}

void init_thread_pool()
{
    pool.task_count = 0;
    pthread_mutex_init(&pool.mutex, NULL);
    pthread_cond_init(&pool.cond, NULL);

    // Create threads in the pool
    for (int i = 0; i < THREAD_POOL_SIZE; i++)
    {
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, worker_thread, NULL);
        pthread_detach(thread_id); // Detach thread to avoid join
    }
}

void add_task_to_pool(task_t *task)
{
    // Lock the mutex to add a task
    pthread_mutex_lock(&pool.mutex);

    // Add task to the queue
    pool.tasks[pool.task_count++] = task;

    // Signal the condition variable to notify a worker
    pthread_cond_signal(&pool.cond);

    // Unlock the mutex
    pthread_mutex_unlock(&pool.mutex);
}