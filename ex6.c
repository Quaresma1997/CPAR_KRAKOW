#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>

#include <errno.h>   
#include <limits.h>  

/* Description of the program: */

/* number of processes */
#define P 12

/* We receive a parameter with then number of subintervals */
int main(int argc, char *argv[])
{
    float sum = 0;
    int N, k, count_num_not_primes = 0, num_primes = 0;
    char *p;
    errno = 0;
    long conv;

    double start,stop;

    int* X;
    int* primes;

    
    
    omp_set_num_threads(P);

    /* If no parameter is passed, return error */
    if (argc > 1) {
        conv = strtol(argv[1], &p, 10);
    }else{
        printf("\nPlease insert a valid number for the sequence or natural numbers!\n\n");
        exit(-1);
    }

    /* Check if the parameter is a valid number */
    if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < 1) {
        printf("\nThe number given is not valid!\n\n");
        exit(-1);
    } else {
        N = conv; 
    }

    X=(int *)malloc(sizeof(int)*N);
    for(int i = 1; i < N; i++)
        X[i] = i;

    
    start=omp_get_wtime();

    #pragma omp parallel for schedule(static) shared(X) reduction(+ : count_num_not_primes)
    for (int i = 2; i <= N / 2; i++){
        printf("Thread %d, index %d\n",omp_get_thread_num(),i);
        if(X[i] != 0)
            for(int j = i * 2; j <= N ; j++){
                #pragma omp critical
                {
                    if(X[i] != 0 && X[j] != 0 && X[j] % X[i] == 0){
                        X[j] = 0;
                        count_num_not_primes++;
                    }
                    
                }
            }   
    }

    stop=omp_get_wtime();
    
    /* Remove the numbers 0 and 1 from the number of primes since they are not primes */
    num_primes = N - count_num_not_primes - 2;
    
    primes=(int *)malloc(sizeof(int)*num_primes);

    int j = 0;

    for(int i = 2; i < N; i++)
        if(X[i] != 0){
            primes[j] = i;
            j++;
        }


    
    printf("\n###### PRIME NUMBERS: ######\n");

    for(int i = 0; i < num_primes; i++)
        printf("Primes[%d] = %d\n", i, primes[i]);

    printf("Number of primes in the first %d natural numbers: %d\n", N, num_primes);

    printf("Time of primes calculation = %lfs\n",stop-start); 

    free(X);
 
  	exit(0);

}
