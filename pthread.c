#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_GOLD "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"

int mineurs, pioches, pelles;

struct params
{
    pthread_mutex_t mutex;
    pthread_cond_t done;
    int id;
};

typedef struct params params_t;

void attendre_outils()
{
    if (pioches > 0 && pelles > 0)
    {
        return;
    }
    sleep(1);
    attendre_outils();
}

void rendre_outils()
{
    pioches = pioches + 1;
    pelles = pelles + 1;
}

void *fils(void *arg)
{
    params_t *params = (params_t *)arg;

    int mineur_id = (*params).id;

    srand(time(NULL) ^ (mineur_id << 16));
    printf(ANSI_COLOR_GREEN "\n>>> Creation du mineur %d <<<\n\n", mineur_id);

    attendre_outils();
    printf("%d Mineurs %d Pioches %d Pelles\n", mineurs, pioches, pelles);
    pthread_mutex_lock(&(*params).mutex);
    pioches = pioches - 1;
    pelles = pelles - 1;
    pthread_mutex_unlock(&(*params).mutex);
    pthread_cond_signal(&(*params).done);

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

    pthread_mutex_lock(&(*params).mutex);
    pioches = pioches + 1;
    pelles = pelles + 1;
    pthread_mutex_unlock(&(*params).mutex);
    pthread_cond_signal(&(*params).done);

    // TODO: Ajouter or au total
    return NULL;
}

int main(int argc, char **argv)
{
    scanf("%d %d %d", &mineurs, &pioches, &pelles);

    printf("%d Mineurs %d Pioches %d Pelles\n", mineurs, pioches, pelles);

    pthread_t threads[mineurs];
    params_t params;
    pthread_mutex_init(&params.mutex, NULL);
    pthread_cond_init(&params.done, NULL);

    pthread_mutex_lock(&params.mutex);

    for (int i = 0; i < mineurs; i++)
    {
        params.id = i;
        pthread_create(&threads[i], NULL, fils, &params);

        pthread_cond_wait(&params.done, &params.mutex);
    }

    for (int i = 0; i < mineurs; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("done!");

    pthread_mutex_destroy(&params.mutex);
    pthread_cond_destroy(&params.done);

    return EXIT_SUCCESS;
}