#include <stdio.h>
#include <stdlib.h>
int input_int()
{
    int a;
    scanf("%d", &a);
    return a;
}

float input_float()
{
    float a;
    scanf("%f", &a);
    return a;
}

int input_char()
{
    char c;
    while (1)
    {
        c = getchar();
        if (c != '\n' && c != '\r' && c != ' ')
            break;
    }

    return (int)c;
}

void print_int(int a)
{
    printf("%d", a);
}

void println_int(int a)
{
    printf("%d\n", a);
}

void print_float(float a)
{
    printf("%f", a);
}

void println_float(float a)
{
    printf("%f\n", a);
}

void print_char(int a)
{
    printf("%c", (char)a);
}

// void neg_idx_except()
// {
//     printf("negative index exception\n");
//     exit(0);
// }

int *allocate(int size)
{
    return (int *)malloc((size_t)size);
}

void deallocate(int *ptr)
{
    free((void *)ptr);
}