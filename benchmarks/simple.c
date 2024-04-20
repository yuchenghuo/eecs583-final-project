#include <stdio.h>

#define SIZE 1600

int main()
{

    int in[SIZE];
    int i,j;

    for (i = 0; i < SIZE; i++)
        in[i] = 0;

    for (j = 100; j < SIZE; j++)
        in[j] += 10;

    for (i = 0; i < SIZE; i++)
        in[i] *= i;

    int sum = 0;
    for (j = 0; j< SIZE; j++)
        sum += in[j];

    fprintf(stdout,"%d\n", sum);

    return 1;
}
