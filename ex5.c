#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <omp.h>
#include <math.h>
#include "time.h"
#include <pthread.h>

#include <errno.h>   
#include <limits.h>  

/* Description of the program: */

/* Number of processes */
#define P 3

/* Number of x variables */
#define N 3

int forwardElim(double mat[N][N+1]); 

void backSub(double mat[N][N+1]); 
  
// function to get matrix content 
void gauss(double mat[N][N+1]) 
{ 
    /* reduction into r.e.f. */
    double start, stop;
    start=omp_get_wtime();
    int singular_flag = forwardElim(mat); 
    stop=omp_get_wtime();

    printf("\n###### RESULTS ######\n");

    printf("Time of sum = %lf\n",stop-start); 
  
    /* if matrix is singular */
    if (singular_flag != -1) 
    { 
        printf("Singular Matrix.\n"); 

        if (mat[singular_flag][N]) 
            printf("Inconsistent System."); 
        else
            printf("May have infinitely many "
                   "solutions."); 
  
        return; 
    } 

    backSub(mat); 
} 
  
// function for elementary operation of swapping two rows 
void swap_row(double mat[N][N+1], int i, int j) 
{ 
    printf("Swapped rows %d and %d\n", i, j); 
  
    for (int k=0; k<=N; k++) 
    { 
        double temp = mat[i][k]; 
        mat[i][k] = mat[j][k]; 
        mat[j][k] = temp; 
    } 
} 
  
// function to print matrix content
void print(double mat[N][N+1]) 
{ 
    for (int i=0; i<N; i++, printf("\n")) 
        for (int j=0; j<=N; j++) 
            printf("%lf ", mat[i][j]); 
  
    printf("\n"); 
} 
  
// function to reduce matrix to R.E.F form (echelon form)
int forwardElim(double mat[N][N+1]) 
{ 
    int flag = -1;
    omp_set_num_threads(P);
    #pragma omp parallel for ordered schedule(static) shared(mat)
    for (int k=0; k<N; k++) 
    { 
        #pragma omp ordered
        {
            //printf("Thread %d, index %d\n",omp_get_thread_num(),k);
            // If there hasn't been found a 0 on the main diagonal, we can still find solution
            if(flag == -1){
                int i_max = k; 
                int v_max = mat[i_max][k]; 

                for (int i = k+1; i < N; i++){
                    if (abs(mat[i][k]) > v_max) 
                        v_max = mat[i][k], i_max = i; 
                }

                if (!mat[i_max][k]){
                    flag = k; // Matrix is singular. So there isn't the possibility to calculate the solution
                }
                
                if(flag == -1){

                    if (i_max != k) 
                        swap_row(mat, k, i_max); 
                    
                    for (int i=k+1; i<N; i++) 
                    { 
                        double f = mat[i][k]/mat[k][k]; 
            
                        for (int j=k+1; j<=N; j++) 
                            mat[i][j] -= mat[k][j]*f; 
            
                        mat[i][k] = 0; 
                    } 
                }
            }
            
            
        }
    } 
    printf("\nMATRIX FINAL STATE:\n\n");
    print(mat); //for matrix state 
    return flag; 
} 
  
// function to calculate the solution
void backSub(double mat[N][N+1]) 
{ 
    double x[N];  // Array of unknown variables

    for (int i = N-1; i >= 0; i--) 
    { 
        x[i] = mat[i][N]; 
  
        for (int j=i+1; j<N; j++) 
        { 
            x[i] -= mat[i][j]*x[j]; 
        } 
  
        x[i] = x[i]/mat[i][i]; 
    } 
  
    printf("\nSolution for the system:\n"); 
    for (int i=0; i<N; i++) 
        printf("x[%d] = %lf\n", i, x[i]); 
} 

int main() 
{ 
    /* input matrix */
    double mat[N][N+1] = {{1.0, 0.0, 0.0, 7.0}, 
                          {3.0, 2.0, 0.0, 27.0}, 
                          {2.0, 4, 8.0, 34.0} 
                         }; 
  
    printf("Matrix initial state:\n");
    print(mat);
    gauss(mat); 
  
    return 0; 
}