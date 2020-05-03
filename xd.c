#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>    // fork()
#include <sys/wait.h>  // waitpid()
#include <sys/types.h> // pid type
#include <time.h>      // time pour rand()
#include <sys/shm.h>   // Pour la zone de memoire partagee
#include <sys/sem.h>   // pour utiliser les semaphores
#include <signal.h>    // Pour envoyer des signaux
#include <sys/mman.h>
#include <string.h>

#define ATTENTE 0
#define TRAVAILLE 1

int sem_set;
int *shpioches, *shpelles;

int main()
{
    int mineurs, pioches, pelles;
    scanf("%d %d %d", &mineurs, &pioches, &pelles);

    shpioches = create_shared_memory(sizeof(int));
    shpelles = create_shared_memory(sizeof(int));

    memcpy(shpioches, &pioches, sizeof(int));
    memcpy(shpelles, &pelles, sizeof(int));

    // Creation des Sémaphores, le dernier est le Mutex
    sem_set = semget(IPC_PRIVATE, mineurs + 1, IPC_CREAT | 0660);

    if (sem_set == -1) // Erreur de mémoire partagée.
        exit(1);

    // On initialise tout les semaphores
    for (int i = 0; i < mineurs; i++)
        sem_init(i, 0); // On initialise les semaphores a 0
    sem_init(mineurs, 1);

    return 0;
}

void *create_shared_memory(size_t size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

void sem_init(int sem, int val)
{
    int S;
    S = semctl(sem_set, sem, SETVAL, val);
}

// Opération sur un Sémaphore.
void sem_op(int sem, int sem_op)
{
    struct sembuf sembuf;
    int S;
    sembuf.sem_num = sem;   // Numéro du Sémaphore
    sembuf.sem_op = sem_op; // Operation sur le Sémaphore
    sembuf.sem_flg = 0;     // Options pour l’Operation
    S = semop(sem_set, &sembuf, 1);
}

// Fonction pour faire attendre un Sémaphore.
void sem_wait(int sem)
{
    sem_op(sem, -1);
}

// Envoi d'un signal a un Sémaphore, pour qu'il continue ou non.
void sem_signal(int sem)
{
    sem_op(sem, 1);
}

void fils(int mineur_id)
{
}