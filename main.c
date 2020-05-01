#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main()
{
    srand(time(0));
    printf("or  | r | r2\n============\n");
    for (int i = 0; i < 5; i++)
    {
        int or, r, r2;
        or = rand() % (1000 - 1) + 1;
        r = rand() % (5 - 1) + 1;
        r2 = rand() % (5 - 1) + 1;
        printf("%d | %d | %d\n------------\n", or, r, r2);
    }

    return 0;
}