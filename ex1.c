#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include "time.h"
#include "mpi.h"

enum LiftState {IDLE, DOORS_OPEN, MOVING};
enum PassengerState {START, INSIDE_ELEVATOR, END};
enum Direction {NONE, UP, DOWN};

#define NUM_FLOORS 11

/*
The Elevator algorihtm

As long as there’s someone inside or ahead of the elevator who wants to go in the current direction, keep heading in that direction.
Once the elevator has exhausted the requests in its current direction, switch directions if there’s a request in the other direction. Otherwise, stop and wait for a call.

If its MOVING and stops on a floor to let passenger get out. passengers on that floor for the same direction can enter (without calling it) and press their buttons
*/

/* CASE:
    Process 3 on floor 7 and process 4 on floor 10. Lift Idle on 10. P3 calls it to go to floor 0, P4 enter the lift to go to floor 3.
    The lift picks up P3, and then executes request from P4. After that executes request from P3 and goes to floor 0.
*/

/* Case2:
    4 processes on floor 2 and P5 on floor 7. Their requests in order: 3 0 5 7 10. The lift goes up until 7 with P2 in it, goes down until 0 to drop off P2 and
    only then goes up until floor 7 to pick up P5 and leave him on floor 10.
*/

/* CUrrently we have a sleep in the end of the loop to allow us to interpret the prints more easily */

typedef struct {
    enum LiftState state;
    enum Direction direction;
    int curFloor;
    int destFloor;
    int num_requests;
    int passengersLeft;
    int* requests;
    int end;
} Lift;

typedef struct {
    enum PassengerState state;
	enum Direction direction;
    int destFloor;
    int curFloor;
} Passenger;


enum Direction getDirection(int curFloor, int destFloor){
    return (curFloor > destFloor ? DOWN : UP);
}

int getNewFloor(int curFloor, enum Direction direction){
    return curFloor + (direction == UP ? 1 : -1);
}

/* Get the new destination for the lift based on the current requests. The destination is aimed at the highest (case of UP direction) or
   lowest (case of DOWN direction) */
int getDestination(int curDest, int* requests, enum Direction direction){
    if(direction == UP){
        if(curDest == NUM_FLOORS){
            for(int i = 0; i < curDest; i++){
                if(requests[i] == 1)
                    return i;
            }
        }else{
            for(int i = NUM_FLOORS; i > curDest; i--){
                if(requests[i] == 1)
                    return i;
            }

            for(int i = 0; i < curDest; i++){
                if(requests[i] == 1)
                    return i;
            }
        }
    }else{
        if(curDest == 0){
            for(int i = NUM_FLOORS; i > curDest; i--){
                if(requests[i] == 1)
                    return i;
            }
        }else{
            for(int i = 0; i < curDest; i++){
                if(requests[i] == 1)
                    return i;
            }

            for(int i = NUM_FLOORS; i > curDest; i--){
                if(requests[i] == 1)
                    return i;
            }
        }
    }
}

Lift lift_function(Lift lift){
    
    
    if(lift.state == IDLE ){
        if(lift.num_requests > 0){
            
            lift.direction = getDirection(lift.curFloor, lift.destFloor);
            lift.state = MOVING;
        }
    }else{
        
        // LIFT moving or open doors

        lift.curFloor = getNewFloor(lift.curFloor, lift.direction);
        if(lift.requests[lift.curFloor] == 1){
            // Lift arrived at a requested floor
            // open doors
            lift.state = DOORS_OPEN;

            lift.requests[lift.curFloor] = 0;
            lift.num_requests--;

            if(lift.num_requests == 0){
                lift.state = IDLE;
                lift.direction = NONE;
                lift.destFloor = -1;
            }else{
                if(lift.curFloor == lift.destFloor){
                    lift.destFloor = getDestination(lift.destFloor, lift.requests, lift.direction);
                    lift.direction = getDirection(lift.curFloor, lift.destFloor);
                }
            }
        }else{
            lift.state = MOVING;
        }
    }
    return lift;
}

int main(int argc, char *argv[])
{
    int world_rank, world_size, next, prev, message, tag = 201;

    double res = 0;
    int x = 0;
    double getNumberTimeTotal = 0;
    double maxtime, mintime;
    const int root=0;

    MPI_Status status;
    int request = 0;


    MPI_Datatype new_type;
	MPI_Datatype type[7] = { MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT };
	int blen[7] = { 1, 1, 1, 1, 1, 1, 1 };
	MPI_Aint disp[7];
	MPI_Aint base, addr;


    /* Start up MPI */

    MPI_Init(&argc, &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Barrier(MPI_COMM_WORLD);  /*synchronize all processes*/

    Lift lift = {IDLE, NONE, 0, -1, 0, world_size - 1, (int *)malloc(sizeof(int)*NUM_FLOORS), 0};  

    for (size_t i = 0; i < NUM_FLOORS; ++i)
        lift.requests[i] = 0;

    Passenger passenger = {START, NONE, -1, -1};

    MPI_Get_address(&lift, &base);
	MPI_Get_address(&(lift.state), &addr);
	disp[0] = addr - base;
	MPI_Get_address(&(lift.direction), &addr);
	disp[1] = addr - base;
    MPI_Get_address(&(lift.curFloor), &addr);
	disp[2] = addr - base;
    MPI_Get_address(&(lift.destFloor), &addr);
	disp[3] = addr - base;
    MPI_Get_address(&(lift.num_requests), &addr);
	disp[4] = addr - base;
    MPI_Get_address(&(lift.passengersLeft), &addr);
	disp[5] = addr - base;
    MPI_Get_address(&(lift.end), &addr);
	disp[6] = addr - base;

	MPI_Type_create_struct(7, blen, disp, type, &new_type);
	MPI_Type_commit(&new_type);


    if(world_rank != root){
        if(passenger.curFloor == -1){
            // Initiate struct
            
            /* Intializes random number generator */
            time_t t;
            /* Intializes random number generator */
            srand((unsigned) time(&t) + world_rank);
            
            passenger.state = START;
            passenger.curFloor = rand() % NUM_FLOORS;
            /*switch(world_rank){
                case 1:
                    passenger.curFloor = 2;
                    passenger.destFloor = 3;
                    passenger.direction = UP;
                    break;
                case 2:
                    passenger.curFloor = 2;
                    passenger.destFloor = 0;
                    passenger.direction = DOWN;
                    break;
                case 3:
                    passenger.curFloor = 2;
                    passenger.destFloor = 7;
                    passenger.direction = UP;
                    break;
                case 4:
                    passenger.curFloor = 2;
                    passenger.destFloor = 5;
                    passenger.direction = UP;
                    break;
                case 5:
                    passenger.curFloor = 7;
                    passenger.destFloor = 10;
                    passenger.direction = UP;
                    break;    
            }*/
        
            printf("Passenger %d is on the floor %d\n", world_rank, passenger.curFloor);
            

             switch(passenger.curFloor){
                 case 0:
                     passenger.direction = UP;
                     break;
                 case NUM_FLOORS - 1:
                     passenger.direction = DOWN;
                     break;
                 default:
                     passenger.direction = (enum Direction)(1 + rand() % 2);
                     break;
             }
            passenger.destFloor = (passenger.direction == UP ? (passenger.curFloor + 1 + rand() % (NUM_FLOORS - passenger.curFloor - 1)) : rand() % passenger.curFloor);

        }
        printf("Passenger %d wants to go to floor %d, direction %d\n", world_rank, passenger.destFloor, passenger.direction);
    }

    do{
        if(world_rank != root) {
            MPI_Recv(&lift, 1, new_type, (world_rank - 1), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(lift.requests, NUM_FLOORS, MPI_INT, (world_rank - 1), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            switch (passenger.state)
            {
                case START:
                    if(lift.state == IDLE){
                        
                        if(lift.destFloor == -1){
                            if(lift.curFloor == passenger.curFloor){
                                lift.destFloor = passenger.destFloor;
                                lift.requests[passenger.destFloor] = 1;
                                lift.num_requests++;
                                passenger.state = INSIDE_ELEVATOR;
                            }else{
                                lift.destFloor = passenger.curFloor;
                                lift.requests[passenger.curFloor] = 1;
                                lift.num_requests++;
                                
                            }
                          
                        }else{
                            
                            if(lift.curFloor == passenger.curFloor){
                                if(lift.requests[passenger.destFloor] == 0){
                                    lift.requests[passenger.destFloor] = 1;
                                    lift.num_requests++;
                                }
                                passenger.state = INSIDE_ELEVATOR;
                            }
                        }
                    }else if(lift.state == DOORS_OPEN){
                        if(lift.direction == passenger.direction && lift.curFloor == passenger.curFloor){
                            passenger.state = INSIDE_ELEVATOR;
                            if(lift.requests[passenger.destFloor] == 0){
                                lift.requests[passenger.destFloor] = 1;
                                lift.num_requests++;
                            }
                        }
                    }
                 
                    break;
                case INSIDE_ELEVATOR:
                    printf("Passenger %d is on the floor %d going to %d\n", world_rank, lift.curFloor, passenger.destFloor);
                    if(lift.state == DOORS_OPEN || lift.state == IDLE){
                        if(lift.curFloor == passenger.destFloor){
                            passenger.state = END;
                            lift.passengersLeft--;
                            printf("PASSENGER %d left the lift on floor %d\n", world_rank, lift.curFloor);
                        }
                    }
                    break;
                default:
                    break;
            }
            
            
        }else{

            
            if(lift.passengersLeft == 0)
                lift.end = 1;
        }
        
        MPI_Send(&lift, 1, new_type, (world_rank + 1) % world_size,
                        0, MPI_COMM_WORLD);
        MPI_Send(lift.requests, NUM_FLOORS, MPI_INT, (world_rank + 1) % world_size,
                        0, MPI_COMM_WORLD);

        
        

        if(world_rank == root) {

            MPI_Recv(&lift, 1, new_type, (world_size - 1), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            MPI_Recv(lift.requests, NUM_FLOORS, MPI_INT, (world_size - 1), MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            
            lift = lift_function(lift);

            if(lift.destFloor == -1)
                printf("LIFT is on the floor %d and is IDLE\n\n", lift.curFloor);
            else    
                printf("LIFT is on the floor %d going to floor %d\n\n", lift.curFloor, lift.destFloor);

        }

        sleep(2);

    }while(lift.end != 1);
    
            


    MPI_Barrier(MPI_COMM_WORLD);
    free(new_type);
    free(lift.requests);
    MPI_Finalize();
    return 0;
}