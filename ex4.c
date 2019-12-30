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
    double sum = 0;
    int N, k;
    char *p;
    errno = 0;
    long conv;

    double start,stop;
    
    omp_set_num_threads(P);

    /* If no parameter is passed, return error */
    if (argc > 1) {
        conv = strtol(argv[1], &p, 10);
    }else{
        printf("\nPlease insert a valid number!\n\n");
        exit(-1);
    }

    /* Check if the parameter is a valid number */
    if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < 1) {
        printf("\nThe number given is not valid!\n\n");
        exit(-1);
    } else {
        N = conv; 
    }

    start=omp_get_wtime();

    #pragma omp parallel for reduction(+ : sum) private(k)
    for(k=1;k<=N;k++){
        sum+=1.0/k;       
        //printf("Sum: %f\n", sum);
        //printf("Thread %d, index %d\n",omp_get_thread_num(),k);
    }

    stop=omp_get_wtime();

    double result = sum - log(N);

    printf("\n###### RESULTS ######\n");

    printf("Result: %f\n",result);
    printf("Time of sum = %lf\n",stop-start); 
 
  	exit(0);

}
