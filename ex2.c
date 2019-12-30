#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <math.h>

#include <errno.h>   
#include <limits.h>  

/* Description of the program: The program receives one argument which is the number of subintervals. 
   After validating the argument given we initialize a struct with the values of the integral given in the exercise description.
   Then the array of threads is created with size n - 1. After that each thread is executed abd the total execution time is measured.
   Finally the final results are calculated */

/* sum of yi */
double sum=0;

/* Number of processes */
#define NUM_THREADS 6

/* initialize mutex */
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

/* struct with integral values */
struct integral_struct {
    double c; // constant multiplying by the integral
    double a;
    double b;
    double h;
    double y0;
    double yn;
    int n;
};


/* Declaration of integral struct */
struct integral_struct integral;

/* Function f */
double f(double x){
    return 1.0 / (1 + pow(x, 2));
}

/* Function yi */
void* yi(void *arg)
{
    int i = (size_t) arg;
    int new_i;
    int local_n = integral.n / NUM_THREADS;

    // Calculate the value of this thread n and i

    if(integral.n % NUM_THREADS != 0){
        int rest = integral.n % NUM_THREADS;
        if((i - 1) / rest < 1){
            local_n++;
            new_i = (i - 1) * local_n + 1;
        }else{
            new_i = rest * (local_n + 1) + (i - rest - 1) * local_n + 1;
        }
    }else{
        new_i = (i - 1) * local_n + 1;
    }

    for(int j = new_i; j < new_i + local_n; j++){
        /* Initiate mutex for the shared varialbe "sum" */
        pthread_mutex_lock( &my_mutex );
        sum += f(integral.a + j * integral.h); // adding the result of yi to the global variable sum
        pthread_mutex_unlock( &my_mutex );
    }
    
    
	
	return (void*)0;
}

/* We receive a parameter with then number of subintervals */
int main(int argc, char *argv[])
{
    size_t i;
    int n;
    char *p;
    errno = 0;
    long conv;

    struct timespec start, finish;
    double elapsed;

    int num_threads;
   
    /* If no parameter is passed, return error */
    if (argc > 1) {
        conv = strtol(argv[1], &p, 10);
    }else{
        printf("\nPlease insert a valid number of subintervals!\n\n");
        exit(-1);
    }

    /* Check if the parameter is a valid number */
    if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < 1) {
        printf("\nThe number of subintervals given is not valid!\n\n");
        exit(-1);
    } else {
        n = conv;    
    }

    /* Initializing integral values */

    integral.a = 0.0;
    integral.b = 1;
    integral.c = 4;

    integral.h = (integral.b - integral.a) / n;
    integral.y0 = f(integral.a);
    integral.yn = f(integral.b);

    integral.n = n - 1;

    /* Prints to check integral values and calculations */
    /* printf("\n###### INTEGRAL VALUES ######\n\n");
    printf("A: %f\n", integral.a);
    printf("B: %f\n", integral.b);
    printf("C: %f\n", integral.c);
    printf("H: %f\n", integral.h);
    printf("Y0: %f\n", integral.y0);
    printf("YN: %f\n", integral.yn); */
    

    /* Creating array of threads */

    pthread_t threads[NUM_THREADS];

    
 
    /* Creating threads and passing i +1 as the yi function argument */
    for (i=0; i < NUM_THREADS; i++) {
        errno = pthread_create(&threads[i], NULL, yi, (void*)(i + 1));
        if (errno) {
                perror("pthread_create");
                return EXIT_FAILURE;
        }
    }

    /* Initialize the time at the beggining of the execution of the threads */

    clock_gettime(CLOCK_MONOTONIC, &start);

    /* Joining threads */
    for( i = 0; i < NUM_THREADS; i++ )	
		pthread_join( threads[i], NULL);

    clock_gettime(CLOCK_MONOTONIC, &finish);

    /* Calculate the execution time */

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1.0e9;

    /* Calculate the integral result */
    double integral_result = integral.h * (((integral.y0 + integral.yn) / 2) + sum);

    /* Calculate the final result */
    double final_result = integral.c * integral_result;

    printf("\n###### RESULTS ######\n");

    printf("\nSum of yi: %f\n",sum);

    printf("\nIntegral result: %f\n",integral_result);

  	printf("\nFinal result: %f\n",final_result);

    printf("\nExecution time: %fs\n\n", elapsed);

  	exit(0);

}
