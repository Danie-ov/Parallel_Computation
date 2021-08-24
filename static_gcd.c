#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "/usr/lib/openmpi/include/mpi.h"

enum {ROOT, N = 10};

int* readPairs(int numOfPairs)
{
    int *arr, line = 2;
    char buffer[250], ch;
    arr = (int*)malloc(sizeof(int)*numOfPairs*2);
    for(int i=0; i<numOfPairs; i++)
    {
        fgets(buffer, 250, stdin);
        sscanf(buffer, "%d %d%c", &arr[i*2], &arr[i*2+1], &ch);
        if(ch != '\n')
        {
            printf("‫‪illegal‬‬ ‫‪input‬‬ ‫‪at‬‬ ‫‪line‬‬ %d\n", line);
            free(arr);
            return NULL;
        }
        line++;
    }
    return arr;
}

int findGCD(int num1, int num2)
{
    if (num2 == 0)
        return num1;

    return findGCD(num2, num1 % num2);
}

void calculateGcdArr(int* resArr, int* arr, int size)
{
    int i;
    for(i=0; i<size; i++)
        resArr[i] = findGCD(arr[i*2], arr[i*2+1]); 
    
}

void printGcdArr(int* arr1, int* arr2, int size)
{
    int i;
    for(i=0; i<size; i++)
        printf("%d\t%d\tgcd:%d\n", arr2[i*2], arr2[i*2+1], arr1[i]);
}

int main(int argc, char* argv[])
{
    int rank, numProcs, numOfPairs, jobsDone = 0;
    char buffer[250];
    double t;
    int *arr, workGcdArr[N*2], *gcdArr, tempGcdArr[N];
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if(rank == ROOT)
    {
        fgets(buffer, 250, stdin);
        numOfPairs = atoi(buffer);
        arr = readPairs(numOfPairs);
        if(!arr)
        {
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
            exit(EXIT_FAILURE);
        }
        t = MPI_Wtime();
        gcdArr = (int*)calloc(numOfPairs,sizeof(int));
    }
    MPI_Bcast(&numOfPairs, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    while(jobsDone + N*numProcs < numOfPairs)
    {
        MPI_Scatter(arr+jobsDone, N*2, MPI_INT, workGcdArr, N*2, MPI_INT, ROOT, MPI_COMM_WORLD);
        calculateGcdArr(tempGcdArr, workGcdArr, N);
        MPI_Gather(tempGcdArr, N, MPI_INT, gcdArr+jobsDone, N, MPI_INT, ROOT, MPI_COMM_WORLD);
        jobsDone += N*numProcs;
    }
    if(rank == ROOT)
    {
        if(numOfPairs > jobsDone)
            calculateGcdArr(gcdArr+jobsDone, arr+jobsDone*2, numOfPairs-jobsDone);

        printf("Runtime: %lf\n", MPI_Wtime()-t); 
        printGcdArr(gcdArr, arr, numOfPairs);
        free(arr);
        free(gcdArr);
    }
    MPI_Finalize();
    return EXIT_SUCCESS;
}