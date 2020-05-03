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

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_GOLD "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"

void sem_init(int sem, int val);
void sem_op(int sem, int sem_op);
void sem_wait(int sem);
void sem_signal(int sem);
_Bool travail_mineur(int m);
void commence_travail(int mineur_id);
void attente_travail(int mineur_id);
void stop_travail(int mineur_id);
void fils(int mineur_id);
void *create_shared_memory(size_t size);

int sem_set;
int mineurs;
int *shpioches, *shpelles;

int main()
{
    printf(ANSI_COLOR_RESET "Combien de mineurs , pioches et pelles ?\n");
    int pioches, pelles;
    scanf("%d %d %d", &mineurs, &pioches, &pelles);

    shpioches = create_shared_memory(sizeof(int));
    shpelles = create_shared_memory(sizeof(int));

    memcpy(shpioches, &pioches, sizeof(int));
    memcpy(shpelles, &pelles, sizeof(int));

    printf(ANSI_COLOR_RESET "---   %d Mineurs | %d Pioches | %d Pelles   ---\n", mineurs, pioches, pelles);

    // Creation des Sémaphores, le dernier est le Mutex
    sem_set = semget(IPC_PRIVATE, mineurs + 1, IPC_CREAT | 0660);

    if (sem_set == -1) // Erreur de mémoire partagée.
        exit(1);

    // On initialise tout les semaphores
    for (int i = 0; i < mineurs; i++)
        sem_init(i, 0); // On initialise les semaphores a 0
    sem_init(mineurs, 1);

    // Création des différents mineurs
    for (int num_processus = 1; num_processus <= mineurs; num_processus++)
    {
        int pid = fork();
        if (pid < 0)
            exit(1);
        if (pid == 0)
            fils(num_processus);
    }

    // Attente de la terminaison de tous les processus fils
    for (int i = 1; i <= mineurs; i++)
        waitpid(-1, NULL, 0);

    return 0;
}

void fils(int mineur_id)
{
    srand(time(NULL) ^ (getpid() << 16));

    printf(ANSI_COLOR_GREEN "\n>>> Creation du mineur %d (%d) <<<\n\n", mineur_id, getpid());

    int r, r2, or, h;
    or = rand() % (1000 - 1) + 1;
    r = rand() % (5 - 1) + 1;
    r2 = rand() % (5 - 1) + 1;

    printf(ANSI_COLOR_BLUE "Mineur %d > ", mineur_id);
    printf(ANSI_COLOR_RESET "Attends %dh avant de travailler.\n", r);
    sleep(r);
    attente_travail(mineur_id);

    printf(ANSI_COLOR_BLUE "Mineur %d > ", mineur_id);
    printf(ANSI_COLOR_RESET "Travaille (encore %dh).\n", r2);
    sleep(r);

    printf(ANSI_COLOR_GOLD
           "\n---   Mineur %d > %dg d'Or Extrait   ---\n\n",
           mineur_id,
           or);

    stop_travail(mineur_id);
}

// Initialisation d'un Sémaphore.
void sem_init(int sem, int val)
{
    int S;
    S = semctl(sem_set, sem, SETVAL, val);
}

// Opération sur un Sémaphore.
void sem_op(int sem, int sem_op_id)
{
    struct sembuf sembuf;
    int S;
    sembuf.sem_num = sem;      // Numéro du Sémaphore
    sembuf.sem_op = sem_op_id; // Operation sur le Sémaphore
    sembuf.sem_flg = 0;        // Options pour l’Operation
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

// Vérifie si un certain mineur peut travailler.
_Bool travail_mineur(int m)
{
    return *shpioches > 0 && *shpelles > 0;
}

// Commence le travail d'un Mineur
void commence_travail(int mineur_id)
{
    sem_signal(mineur_id);
    *shpioches--;
    *shpelles--;
    printf(ANSI_COLOR_BLUE "Mineur %d > ", mineur_id);
    printf(ANSI_COLOR_RESET "Prends 1 Pelle et 1 Pioche (Reste %d pelles et %d pioches).\n", *shpelles, *shpioches);
}

void attente_travail(int mineur_id)
{
    if (travail_mineur(mineur_id))
        commence_travail(mineur_id);
    sem_signal(mineurs);
    sem_wait(mineur_id);
}

void stop_travail(int mineur_id)
{
    sem_signal(mineurs);
    *shpioches++;
    *shpelles++;
    exit(0);
}

void *create_shared_memory(size_t size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}