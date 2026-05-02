#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define MAX_XS 50 

void swapvals(int *one, int *two) {
    int tmp = *two;
    *two = *one;
    *one = tmp;
}

void bubble(int *xs, int *passes)
{   
    // track the sorted count -- bubbling largest item to the end of the array.
    int pass = 0;
    int sorted = 0;
    while(!sorted && pass < MAX_XS) {
        sorted = 1;
        // iterate with one pointer on the array, comparing to the next item and swapping when ptr[n] > p[n+1]
        for (int i = 0; i < MAX_XS - pass; ++i) {
            if (xs[i] > xs[i+1]) {
                swapvals(&xs[i], &xs[i+1]);
                sorted = 0;
            }
        }
        pass++;
    }
    *passes = pass;
}

void insertion(int *vals, int *passes)
{
    int i;
    int j;
    int pass = 0;
    for (i = 1; i < MAX_XS; ++i)
    {
        j = i - 1;
        do
        {
            if (vals[j + 1] > vals[j]) break;
            
            swapvals(&vals[j+1], &vals[j]);
            j--;
        } while (j >= 0);
        pass++;
    }
    *passes = pass;
}

int compareint(const void *int1, const void *int2)
{
    return *(const int *)int1 - *(const int *)int2;
}

int partition(int *vals, int i, int j)
{
    int p[3];
    int pval;
    // TODO why not pick the median of 3 random values instead of indices?
    p[0] = (rand() % (j - i + 1)) + i;
    p[1] = (rand() % (j - i + 1)) + i;
    p[2] = (rand() % (j - i + 1)) + i;
    qsort(p, 3, sizeof(int), compareint);
    pval = vals[p[1]];

    i--;
    j++;
    while (1)
    {
        do
        {
            j--;
        } while (vals[j] > pval);

        do
        {
            i++;
        } while (vals[i] < pval);

        if (i >= j)
        {
            break;
        }
        else
        {
            swapvals(&vals[i], &vals[j]);
        }
    }
    return j;
}

int qksort(int *vals, int i, int j)
{
    int k;
    while (i < j)
    {
        if ((k = partition(vals, i, j)) < 0)
        {
            return -1;
        }
        // recursively sort the left partition
        if (qksort(vals, i, k) < 0)
        {
            return -1;
        }
        // iterate and sort the right partition
        i = k + 1;
    }
    return 0;
}

void qsorti(int *vals, int *passes)
{
    int rc;
    rc = qksort(vals, 0, MAX_XS - 1);
    printf("qksort rc = %d\n", rc);
    *passes = 0;
}

void printvals(int len, int arr[len]) {
    printf("{ ");
    for (int i = 0; i < len; ++i) {
        printf("%d ", arr[i]);
    }
    printf("}\n");
}

int main(void) {
    void (*sorts[3])(int *vals, int *passes) = {bubble, insertion, qsorti};
    char *sortnames[3] = {"Bubble", "Insertion", "Quick"};
    int passes;
    int validated = -1;
    int xs[MAX_XS] = {0};
    clock_t start;
    double dur;
    srand(time(NULL));

    for (int i = 0; i < sizeof(sorts) / sizeof(sorts[0]); ++i)
    {
        //gen randoms
        for (int i = 0; i < MAX_XS; ++i) {
            xs[i] = rand() % MAX_XS;
        }
        
        printf("[%s sort]\nNumber of elements: %d\n", sortnames[i], MAX_XS);
        start = clock();
        sorts[i](xs, &passes);
        dur = (double)(clock() - start) / CLOCKS_PER_SEC;
        // validate
        validated = -1;
        for (int i = 0; i < MAX_XS - 1; ++i) 
        {
            if (xs[i] > xs[i+1]) 
            {
                validated = i;
                break;
            }
        }
        if (validated >= 0) {
            fprintf(stderr, "Validation failure: idx %d", validated);
            printvals(MAX_XS, xs);
        }
        else
        {
            printf("number of passes: %d\nduration: %f\n", passes, dur);
        }
    }
}
