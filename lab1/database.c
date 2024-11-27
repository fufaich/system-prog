#include "database.h"
#include "btree.h"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SHM_KEY 12345  // Ключ для shared memory
#define SEM_KEY 67890  // Ключ для семафоров
#define SHM_SIZE 1024  // Размер shared memory

// Глобальные идентификаторы
static int shm_id;
static int sem_id;
static BTree* shared_tree;

void lock_semaphore() {
    struct sembuf operation = { 0, -1, 0 };
    semop(sem_id, &operation, 1);
}

void unlock_semaphore() {
    struct sembuf operation = { 0, 1, 0 };
    semop(sem_id, &operation, 1);
}

int createdb() {
    shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Failed to create shared memory");
        return errno;
    }

    sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Failed to create semaphore");
        shmctl(shm_id, IPC_RMID, NULL);
        return errno;
    }

    semctl(sem_id, 0, SETVAL, 1);

    shared_tree = (BTree*)shmat(shm_id, NULL, 0);
    if (shared_tree == (void*)-1) {
        perror("Failed to attach shared memory");
        return errno;
    }

    // Инициализация B-дерева
    shared_tree->root = create_node(1);

    shmdt(shared_tree);
    return 0;
}

int add(int key, const char* value) {
    lock_semaphore();

    shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    shared_tree = (BTree*)shmat(shm_id, NULL, 0);

    insert_key(shared_tree, key, value);

    shmdt(shared_tree);
    unlock_semaphore();
    return 0;
}

const char* search(int key) {
    const char* result = NULL;

    lock_semaphore();
    shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    shared_tree = (BTree*)shmat(shm_id, NULL, 0);

    result = search_key(shared_tree, key);

    shmdt(shared_tree);
    unlock_semaphore();
    return result;
}

int delete(int key) {
    lock_semaphore();

    shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    shared_tree = (BTree*)shmat(shm_id, NULL, 0);

    delete_key(shared_tree, key);

    shmdt(shared_tree);
    unlock_semaphore();
    return 0;
}
