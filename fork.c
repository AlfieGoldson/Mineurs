#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/mman.h>
#include <string.h>

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_GOLD "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"

int mineurs;
int *shpioches, *shpelles, *shor_total;

void *create_shared_memory(size_t size)
{
    return mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
}

void attendre_outils()
{
    if (*shpioches > 0 && *shpelles > 0)
    {
        *shpioches = *shpioches - 1;
        *shpelles = *shpelles - 1;
        return;
    }

    sleep(1);
    attendre_outils();
}

void rendre_outils()
{
    *shpioches = *shpioches + 1;
    *shpelles = *shpelles + 1;
}

int fils(int mineur_id)
{
    srand(time(NULL) ^ (getpid() << 16));

    attendre_outils();

    printf(ANSI_COLOR_GREEN "\n>>> Creation du mineur %d (%d) <<<\n\n", mineur_id, (int)getpid());

    int r, r2, or_mineur;
    or_mineur = rand() % (1000 - 1) + 1;
    r = rand() % (5 - 1) + 1;
    r2 = rand() % (5 - 1) + 1;

    printf(ANSI_COLOR_BLUE "Mineur %d > ", mineur_id);
    printf(ANSI_COLOR_RESET "Attend %dh avant de travailler.\n", r);
    sleep(r);

    printf(ANSI_COLOR_BLUE "Mineur %d > ", mineur_id);
    printf(ANSI_COLOR_RESET "Travaille (encore %dh).\n", r2);
    sleep(r2);

    printf(ANSI_COLOR_GOLD
           "\n>>> Mineur %d > %dg d'Or Extrait <<<\n\n",
           mineur_id,
           or_mineur);

    rendre_outils();
    *shor_total = *shor_total + or_mineur;
    exit(0);
}

int main()
{
    printf(ANSI_COLOR_RESET "Combien de Mineurs , Pioches et Pelles ?\n");
    int pioches, pelles;
    scanf("%d %d %d", &mineurs, &pioches, &pelles);

    shpioches = create_shared_memory(sizeof(int));
    shpelles = create_shared_memory(sizeof(int));
    shor_total = create_shared_memory(sizeof(int));

    memcpy(shpioches, &pioches, sizeof(int));
    memcpy(shpelles, &pelles, sizeof(int));

    printf("%d Mineurs - %d Pioches - %d Pelles\n", mineurs, pioches, pelles);

    for (int i = 0; i < mineurs; i++)
    {
        int pid = (int)fork();
        if (pid < 0)
            exit(1);
        if (pid == 0)
        {
            fils(i);
        }
    }

    *shor_total = 0;

    for (int i = 0; i < mineurs; i++)
        wait(NULL);

    printf(ANSI_COLOR_GOLD "[Or Total: %d]\n", *shor_total);

    return 0;
}