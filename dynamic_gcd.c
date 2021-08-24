#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "/usr/lib/openmpi/include/mpi.h"

enum {ROOT, N = 10};
enum tags { WORK, STOP };

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

void masterProcess(int numProcs, int chunk)
{
    int *arr, *gcdArr;
    int numOfPairs, workerId, tag = WORK, remainder = 0, jobSent = 0;
    char buffer[250];
    double time;
    MPI_Status status;

    fgets(buffer, 250, stdin);
    numOfPairs = atoi(buffer);
    arr = readPairs(numOfPairs);
    if (!arr)
    {
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }
    time = MPI_Wtime();
    gcdArr = (int*)malloc(sizeof(int)*numOfPairs);
    
    while (tag != STOP)
    {
        if (numOfPairs <= jobSent + (numProcs - 1) * chunk)
        {
            tag = STOP;
            chunk = (numOfPairs - jobSent) / (numProcs - 1);
            remainder = (numOfPairs - jobSent) % (numProcs - 1);
            if (remainder != 0)
                calculateGcdArr(gcdArr+(numOfPairs-remainder), arr+(numOfPairs-remainder)*2, remainder);

        }
        else
            tag = WORK;

        for(workerId = 1; workerId < numProcs; workerId++)
        {
            MPI_Send(&chunk, 1, MPI_INT, workerId, tag, MPI_COMM_WORLD);
            MPI_Send(arr + (jobSent + (workerId - 1) * chunk)*2, chunk*2, MPI_INT, workerId, tag, MPI_COMM_WORLD);
        }
        
        for (workerId = 1; workerId < numProcs; workerId++)
            MPI_Recv(gcdArr + jobSent + (workerId - 1) * chunk, chunk, MPI_INT, workerId, tag, MPI_COMM_WORLD, &status);
        
        jobSent += (numProcs - 1) * chunk;
    }

    printf("Run time: %lf\n", MPI_Wtime() - time);
    printGcdArr(gcdArr, arr, numOfPairs);
    free(arr);
    free(gcdArr);
}

void workerProcess()
{
    int *gcdNumbers, *arr;
    int chunk, tag;
    MPI_Status status;

    do
    {
        MPI_Recv(&chunk, 1, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        gcdNumbers = (int*)malloc(chunk * sizeof(int));
        arr = (int*)malloc(sizeof(int)*chunk*2);

        MPI_Recv(arr, chunk*2, MPI_INT, ROOT, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        tag = status.MPI_TAG;
        calculateGcdArr(gcdNumbers, arr, chunk);
        MPI_Send(gcdNumbers, chunk, MPI_INT, ROOT, tag, MPI_COMM_WORLD);

        free(gcdNumbers);
        free(arr);
    } while (tag != STOP);

}

int main(int argc, char* argv[])
{
    int rank, numProcs;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &numProcs);

    if(rank == ROOT)
        masterProcess(numProcs, argc >= 2? atoi(argv[1]):2);
    else
        workerProcess();
    
    MPI_Finalize();
    return EXIT_SUCCESS;
}