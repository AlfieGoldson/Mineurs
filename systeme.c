#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>    // fork()
#include <sys/wait.h>  // waitpid()
#include <sys/types.h> // pid type
#include <time.h>      // time pour rand()
#include <sys/shm.h>   // Pour la zone de memoire partagee
#include <sys/sem.h>   // pour utiliser les semaphores
#include <signal.h>    // Pour envoyer des signaux

#define ATTENTE 0
#define TRAVAILLE 1

int sem_set;
char *etat_mineur;
int mineurs, pioches, pelles;

void sem_init(int sem, int val);
void sem_op(int sem, int sem_op);
void sem_wait(int sem);
void sem_signal(int sem);
_Bool travail_mineur(int m);
void commence_travail(int mineur_id);
void attente_travail(int mineur_id);
void stop_travail(int mineur_id);
void fils(int mineur_id);

int main()
{
    printf("Combien de mineurs , pioches et pelles ?\n");
    scanf("%d %d %d", &mineurs, &pioches, &pelles);

    printf("---   %d Mineurs | %d Pioches | %d Pelles   ---\n", mineurs, pioches, pelles);

    // Création Memoire Partagée
    int shmid = shmget(IPC_PRIVATE, mineurs, IPC_CREAT | 0660);
    etat_mineur = (char *)shmat(shmid, NULL, 0);

    int pid;

    // Creation des Sémaphores, le dernier est le Mutex
    sem_set = semget(IPC_PRIVATE, mineurs + 1, IPC_CREAT | 0660);
    if (sem_set == -1)
    {
        printf("Erreur de mémoire partagée.\n");
        exit(1);
    }

    // On initialise tout les semaphores
    int i;
    for (i = 0; i < mineurs; i++)
        sem_init(i, 0); // On initialise les semaphores a 0
    sem_init(i, 1);

    // A la creation tout les mineurs attendent pour travailler
    for (i = 0; i < mineurs; i++)
        etat_mineur[i] = ATTENTE;

    // Création des différents mineurs
    int num_processus;
    for (num_processus = 1; num_processus <= mineurs; num_processus++)
    {
        pid = fork();
        if (pid == 0)
            break;
    }

    if (pid == 0)
        fils(num_processus);
    else if (pid < 0)
    {
        printf("Erreur de fork\n");
        exit(1);
    }
    else
        for (int i = 1; i <= mineurs; i++)
            waitpid(-1, NULL, 0); // On attend que tout les processus fils se terminent

    return 0;
}

// Initialisation d'un Sémaphore.
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

// Vérifie si un certain mineur peut travailler.
_Bool travail_mineur(int m)
{
    return etat_mineur[m] == ATTENTE && pioches > 0 && pelles > 0;
}

// Commence le travail d'un Mineur
void commence_travail(int mineur_id)
{
    etat_mineur[mineur_id] = TRAVAILLE;
    sem_signal(mineur_id);
    pioches--;
    pelles--;
    printf("Mineur %d > Prends 1 Pelle et 1 Pioche (Reste %d pelles et %d pioches).\n", mineur_id, pelles, pioches);
}

void attente_travail(int mineur_id)
{
    sem_wait(mineurs);
    etat_mineur[mineur_id] = ATTENTE;
    if (travail_mineur(mineur_id))
        commence_travail(mineur_id);
    sem_signal(mineurs);
    sem_wait(mineur_id);
}

void stop_travail(int mineur_id)
{
    sem_wait(mineurs);
    etat_mineur[mineur_id] = ATTENTE;
    sem_signal(mineurs);
    pioches++;
    pelles++;
    exit(0);
}

void fils(int mineur_id)
{
    srand(time(NULL) ^ (getpid() << 16));

    printf("\n---\n| Creation du mineur %d (%d)\n---\n\n", mineur_id, getpid());

    for (int i = 1; i < mineurs; i++)
    {
        int r, r2, or, h;
        or = rand() % (1000 - 1) + 1;
        r = rand() % (5 - 1) + 1;
        r2 = rand() % (5 - 1) + 1;

        printf("Mineur %d > Attends %dh avant de travailler.\n", mineur_id, r);
        sleep(r);
        attente_travail(mineur_id);

        printf(
            "Mineur %d > Travaille (encore %dh).\n", mineur_id, r2);
        sleep(r);

        printf(
            "\n---   Mineur %d > %dg d'Or Extrait   ---\n\n",
            mineur_id,
            or);

        stop_travail(mineur_id);
    }
}