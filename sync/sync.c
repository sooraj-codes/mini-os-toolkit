#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#endif

#include "sync.h"

#define BUFFER_SIZE 5
#define ITEMS_TO_PRODUCE 10
#define PHILOSOPHER_COUNT 5

#ifdef _WIN32
typedef HANDLE sync_thread_t;
typedef HANDLE sync_sem_t;
typedef CRITICAL_SECTION sync_mutex_t;
typedef DWORD sync_thread_return_t;
#define SYNC_THREAD_CALL WINAPI

static int sync_sem_init(sync_sem_t *semaphore, unsigned int value) {
    *semaphore = CreateSemaphore(NULL, (LONG)value, BUFFER_SIZE, NULL);
    return *semaphore != NULL;
}

static void sync_sem_wait(sync_sem_t *semaphore) {
    WaitForSingleObject(*semaphore, INFINITE);
}

static void sync_sem_post(sync_sem_t *semaphore) {
    ReleaseSemaphore(*semaphore, 1, NULL);
}

static void sync_sem_destroy(sync_sem_t *semaphore) {
    CloseHandle(*semaphore);
}

static int sync_mutex_init(sync_mutex_t *mutex) {
    InitializeCriticalSection(mutex);
    return 1;
}

static void sync_mutex_lock(sync_mutex_t *mutex) {
    EnterCriticalSection(mutex);
}

static void sync_mutex_unlock(sync_mutex_t *mutex) {
    LeaveCriticalSection(mutex);
}

static void sync_mutex_destroy(sync_mutex_t *mutex) {
    DeleteCriticalSection(mutex);
}

static void sync_sleep(unsigned int milliseconds) {
    Sleep(milliseconds);
}
#else
typedef pthread_t sync_thread_t;
typedef sem_t sync_sem_t;
typedef pthread_mutex_t sync_mutex_t;
typedef void *sync_thread_return_t;
#define SYNC_THREAD_CALL

static int sync_sem_init(sync_sem_t *semaphore, unsigned int value) {
    return sem_init(semaphore, 0, value) == 0;
}

static void sync_sem_wait(sync_sem_t *semaphore) {
    sem_wait(semaphore);
}

static void sync_sem_post(sync_sem_t *semaphore) {
    sem_post(semaphore);
}

static void sync_sem_destroy(sync_sem_t *semaphore) {
    sem_destroy(semaphore);
}

static int sync_mutex_init(sync_mutex_t *mutex) {
    return pthread_mutex_init(mutex, NULL) == 0;
}

static void sync_mutex_lock(sync_mutex_t *mutex) {
    pthread_mutex_lock(mutex);
}

static void sync_mutex_unlock(sync_mutex_t *mutex) {
    pthread_mutex_unlock(mutex);
}

static void sync_mutex_destroy(sync_mutex_t *mutex) {
    pthread_mutex_destroy(mutex);
}

static void sync_sleep(unsigned int milliseconds) {
    usleep(milliseconds * 1000);
}
#endif

typedef struct {
    int items[BUFFER_SIZE];
    int in;
    int out;
    sync_sem_t empty;
    sync_sem_t full;
    sync_mutex_t mutex;
} bounded_buffer_t;

typedef struct {
    bounded_buffer_t *buffer;
    int count;
} producer_args_t;

static sync_thread_return_t SYNC_THREAD_CALL producer(void *argument) {
    producer_args_t *args = (producer_args_t *)argument;

    for (int item = 1; item <= args->count; item++) {
        sync_sem_wait(&args->buffer->empty);
        sync_mutex_lock(&args->buffer->mutex);

        args->buffer->items[args->buffer->in] = item;
        args->buffer->in = (args->buffer->in + 1) % BUFFER_SIZE;
        printf("Producer produced item %d\n", item);

        sync_mutex_unlock(&args->buffer->mutex);
        sync_sem_post(&args->buffer->full);
        sync_sleep(100);
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

static sync_thread_return_t SYNC_THREAD_CALL consumer(void *argument) {
    producer_args_t *args = (producer_args_t *)argument;

    for (int item = 0; item < args->count; item++) {
        int consumed;

        sync_sem_wait(&args->buffer->full);
        sync_mutex_lock(&args->buffer->mutex);

        consumed = args->buffer->items[args->buffer->out];
        args->buffer->out = (args->buffer->out + 1) % BUFFER_SIZE;
        printf("Consumer consumed item %d\n", consumed);

        sync_mutex_unlock(&args->buffer->mutex);
        sync_sem_post(&args->buffer->empty);
        sync_sleep(150);
    }

#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

static int sync_thread_create(sync_thread_t *thread,
                              sync_thread_return_t (SYNC_THREAD_CALL *function)(void *),
                              void *argument) {
#ifdef _WIN32
    *thread = CreateThread(NULL, 0, function, argument, 0, NULL);
    return *thread != NULL;
#else
    return pthread_create(thread, NULL, function, argument) == 0;
#endif
}

static void sync_thread_join(sync_thread_t thread) {
#ifdef _WIN32
    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
#else
    pthread_join(thread, NULL);
#endif
}

static void run_producer_consumer(void) {
    bounded_buffer_t buffer = {0};
    producer_args_t args = {&buffer, ITEMS_TO_PRODUCE};
    sync_thread_t producer_thread;
    sync_thread_t consumer_thread;

    if (!sync_sem_init(&buffer.empty, BUFFER_SIZE) ||
        !sync_sem_init(&buffer.full, 0) ||
        !sync_mutex_init(&buffer.mutex)) {
        fprintf(stderr, "Unable to initialize producer-consumer synchronization.\n");
        return;
    }

    printf("\n-- Producer-Consumer Demo --\n");
    if (!sync_thread_create(&producer_thread, producer, &args) ||
        !sync_thread_create(&consumer_thread, consumer, &args)) {
        fprintf(stderr, "Unable to create producer-consumer threads.\n");
        return;
    }

    sync_thread_join(producer_thread);
    sync_thread_join(consumer_thread);
    sync_mutex_destroy(&buffer.mutex);
    sync_sem_destroy(&buffer.empty);
    sync_sem_destroy(&buffer.full);
    printf("Producer-consumer demo complete.\n");
}

typedef struct {
    sync_mutex_t *forks;
    int philosopher;
} philosopher_args_t;

static sync_thread_return_t SYNC_THREAD_CALL philosopher(void *argument) {
    philosopher_args_t *args = (philosopher_args_t *)argument;
    int left = args->philosopher;
    int right = (left + 1) % PHILOSOPHER_COUNT;
    int first = left < right ? left : right;
    int second = left < right ? right : left;

    printf("Philosopher %d is thinking.\n", args->philosopher + 1);
    sync_sleep(100);

    sync_mutex_lock(&args->forks[first]);
    sync_mutex_lock(&args->forks[second]);
    printf("Philosopher %d is eating.\n", args->philosopher + 1);
    sync_sleep(150);
    sync_mutex_unlock(&args->forks[second]);
    sync_mutex_unlock(&args->forks[first]);

    printf("Philosopher %d finished eating.\n", args->philosopher + 1);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

static void run_dining_philosophers(void) {
    sync_mutex_t forks[PHILOSOPHER_COUNT];
    philosopher_args_t args[PHILOSOPHER_COUNT];
    sync_thread_t threads[PHILOSOPHER_COUNT];

    printf("\n-- Dining Philosophers Demo --\n");
    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        if (!sync_mutex_init(&forks[i])) {
            fprintf(stderr, "Unable to initialize philosopher mutexes.\n");
            return;
        }
    }

    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        args[i].forks = forks;
        args[i].philosopher = i;
        if (!sync_thread_create(&threads[i], philosopher, &args[i])) {
            fprintf(stderr, "Unable to create philosopher threads.\n");
            return;
        }
    }

    for (int i = 0; i < PHILOSOPHER_COUNT; i++) {
        sync_thread_join(threads[i]);
        sync_mutex_destroy(&forks[i]);
    }
    printf("Dining philosophers demo complete.\n");
}

void run_sync_menu(void) {
    int choice;

    do {
        printf("\n===== Synchronization Demo =====\n");
        printf("1. Producer-Consumer (Semaphores)\n");
        printf("2. Dining Philosophers (Mutexes)\n");
        printf("0. Return to Main Menu\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid choice.\n");
            while (getchar() != '\n') {
                /* discard invalid input */
            }
            continue;
        }

        switch (choice) {
            case 1:
                run_producer_consumer();
                break;
            case 2:
                run_dining_philosophers();
                break;
            case 0:
                printf("Returning to Main Menu...\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (choice != 0);
}
