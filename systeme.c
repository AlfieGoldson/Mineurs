#include <stdlib.h>
#include <stdio.h>
// fork()
#include <unistd.h>
// waitpid()
#include <sys/wait.h>
// pid type
#include <sys/types.h>
// time pour rand()
#include <time.h>
// Pour la zone de memoire partagee
#include <sys/shm.h>
// pour utiliser les semaphores
#include <sys/sem.h>
// Pour envoyer des signaux
#include <signal.h>

#define ATTENTE 0
#define TRAVAILLE 1

int sem_set;
char *etat_mineur;
int mineurs, pioches, pelles;

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
}

void attente_travail(int mineur_id)
{
    sem_wait(mineurs);
    printf("Le mineur %d prend une pelle et une pioche.\n", mineur_id);
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

    for (int i = 1; i < mineurs; i++)
    {
        int r, r2, or, h;
        or = rand() % (1000 - 1) + 1;
        r = rand() % (5 - 1) + 1;
        r2 = rand() % (5 - 1) + 1;

        h = 0;

        for (int j = 0; j < r; j++)
        {
            sleep(1);
            printf(
                "Mineur %d | h%d > Attends avant de travailler (encore %dh).\n",
                mineur_id,
                h,
                r - j);
            h++;
        }
        attente_travail(mineur_id);

        for (int j = 0; j < r2; j++)
        {
            sleep(1);
            printf(
                "Mineur %d | h%d > Travaille (encore %dh).\n",
                mineur_id,
                h,
                r2 - j);
            h++;
        }

        printf(
            "\n--- Mineur %d | h%d > %dg d'Or Extrait ---\n\n",
            mineur_id,
            h,
            or);

        // printf("Le mineur %d rend une pelle et une pioche\n", m);
        // printf("Le Mineur %d a fini de travailler! (%d)\n", m, getpid());

        stop_travail(mineur_id);
    }
}

int main()
{
    printf("Combien de mineurs , pioches et pelles ?\n");
    scanf("%d %d %d", &mineurs, &pioches, &pelles);

    printf("# %d Mineurs | %d Pioches | %d Pelles\n", mineurs, pioches, pelles);

    int shmid;
    shmid = shmget(IPC_PRIVATE, mineurs, IPC_CREAT | 0660); // Creation memoire partagee
    etat_mineur = (char *)shmat(shmid, NULL, 0);

    int pid;

    // Creation des semaphores, le dernier est le mutex
    sem_set = semget(IPC_PRIVATE, mineurs + 1, IPC_CREAT | 0660);
    if (sem_set == -1)
    {
        printf("Erreur de memoire partagee\n");
        exit(1);
    }

    // On initialise tout les semaphores
    int i;
    for (i = 0; i < mineurs; i++)
        sem_init(i, 0); // On initialise les semaphores a 0
    // printf("%d", i);
    sem_init(i, 1);

    // A la creation tout les mineurs attendent pour travailler
    for (i = 0; i < mineurs; i++)
        etat_mineur[i] = ATTENTE;

    // On cree les differents mineurs
    int num_processus;
    for (num_processus = 1; num_processus <= mineurs; num_processus++)
    {
        pid = fork();
        if (pid == 0)
            break;
    }

    if (pid == 0)
    {
        printf("Creation du mineur %d (%d)\n", num_processus, getpid());
        fils(num_processus);
    }
    else if (pid < 0)
    {
        printf("Erreur de fork\n");
        exit(1);
    }
    else
    {
        for (int i = 1; i <= mineurs; i++)
        {
            waitpid(-1, NULL, 0); // On attend que tout les processus fils se terminent
        }
    }

    return 0;
}
