#include <stdio.h>
#include <math.h>
#include "time.h"
#include "mpi.h"



/* Function to calculate the factorial of a given number */
int factorial(int x){
    if(x == 0 || x == 1) 
        return 1;
    int ret = 1;
    for(int i = 2; i <= x; i++){
        ret *= i;
    }
    return ret;
}

double f(int x, int n){
    //printf("X: %d, N: %d R: %f\n", x, n, pow(x, n) / factorial(n));
    return pow(x, n) / factorial(n);
}


int main(int argc, char *argv[])
{
    int world_rank, world_size, next, prev, message, tag = 201;

    double res = 0;
    int x = 0;
    double getNumberTimeTotal = 0;
    double maxtime, mintime;
    

    /* Start up MPI */

    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Barrier(MPI_COMM_WORLD);  /*synchronize all processes*/
    
    if (world_rank != 0) {
        MPI_Recv(&x, 1, MPI_INT, world_rank - 1, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }else{
 //       do{
            printf("Insert a number (x): \n");
            scanf("%d", &x);
            
//        }while()
        
    }
    double tbeg = MPI_Wtime();
    res+= f(x, world_rank);
    double elapsedTime = MPI_Wtime() - tbeg;

    MPI_Send(&x, 1, MPI_INT, (world_rank + 1) % world_size,
            0, MPI_COMM_WORLD);
            
    
    double totalTime, totalResult;
    MPI_Reduce( &elapsedTime, &totalTime, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );
    MPI_Reduce( &res, &totalResult, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD );

    if (world_rank == 0) {
        MPI_Recv(&x, 1, MPI_INT, world_rank - 1, 0,
                MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf( "Final result: %f\n", totalResult );
        printf( "Total time spent: %fs\n", totalTime );
    }


    MPI_Finalize();
    return 0;
}