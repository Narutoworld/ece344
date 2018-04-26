/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>
#include <synch.h>

/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20

/*
 * Directions: from + to
 */
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

/*
 * Car movement
 */
#define TURN_RIGHT 0
#define GO_STRAIGHT 1
#define TURN_LEFT 2


/*
 *
 * Function Definitions
 *
 */

/*
 *Some global Vars
 */
static struct semaphore* thread_wait;
//lock when 3 regions contain car
struct semaphore *prevent_deadlock;
struct semaphore *NE_SEM;
struct semaphore *SE_SEM;
struct semaphore *SW_SEM;
struct semaphore *NW_SEM;

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

/*
 * Dead lock Discussion
 * Only 1 car in the intersection, Never Happened
 * Only 2 car in the intersection
 *     - Nearby  Never Happened
 *     - Across  Never Happened
 * Only 3 car in the intersection
 *     - ALL cases never happened
 * All 4 car in the intersection
 *     - Happened: if none of the car turn right !!!!
 */


static 
void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

	switch(cardirection){

	    case SOUTH:
            message(APPROACHING, carnumber, cardirection, NORTH);
            P(SE_SEM);
            message(REGION1, carnumber, cardirection, NORTH);

            P(NE_SEM);
            message(REGION2,carnumber,cardirection, NORTH);
            V(SE_SEM);
			
            message(LEAVING,carnumber,cardirection,NORTH);
            V(NE_SEM);
            break;

	    case NORTH:
            message(APPROACHING, carnumber, cardirection, SOUTH);
            P(NW_SEM);
            message(REGION1, carnumber, cardirection, SOUTH);

            P(SW_SEM);
            message(REGION2,carnumber,cardirection, SOUTH);
            V(NW_SEM);
			
            message(LEAVING,carnumber,cardirection,SOUTH);
            V(SW_SEM);
            break;

	    case WEST:
            message(APPROACHING, carnumber, cardirection, EAST);
            P(SW_SEM);
            message(REGION1, carnumber, cardirection, EAST);

	        P(SE_SEM);
            message(REGION2,carnumber,cardirection, EAST);
            V(SW_SEM);
			
            message(LEAVING,carnumber,cardirection,EAST);
            V(SE_SEM);
            break;

	    case EAST:
            message(APPROACHING, carnumber, cardirection, WEST);
            P(NE_SEM);
            message(REGION1, carnumber, cardirection, WEST);

            P(NW_SEM);
            message(REGION2,carnumber,cardirection, WEST);
            V(NE_SEM);
			
            message(LEAVING,carnumber,cardirection, WEST);
            V(NW_SEM);
            break;
        default:
            assert(0 && "UNKNOWN ERROR");
            break;
    }
	V(prevent_deadlock);
		

}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

	switch (cardirection){
	    case NORTH:
            message(APPROACHING, carnumber, cardirection, EAST);
            P(NW_SEM);
            message(REGION1, carnumber, cardirection, EAST);
		
            P(SW_SEM);
            message(REGION2, carnumber, cardirection, EAST);
            V(NW_SEM);
		
            P(SE_SEM);
            message(REGION3, carnumber, cardirection, EAST);
            V(SW_SEM);
		
            message(LEAVING, carnumber, cardirection, EAST);
            V(SE_SEM);
            break;

	    case SOUTH:
            message(APPROACHING, carnumber, cardirection, WEST);
            P(SE_SEM);
            message(REGION1, carnumber, cardirection, WEST);
		
            P(NE_SEM);
            message(REGION2, carnumber, cardirection, WEST);
            V(SE_SEM);
		
            P(NW_SEM);
            message(REGION3, carnumber, cardirection, WEST);
            V(NE_SEM);
		
            message(LEAVING, carnumber, cardirection, WEST);
            V(NW_SEM);
            break;

	    case WEST:
            message(APPROACHING, carnumber, cardirection, NORTH);
            P(SW_SEM);
            message(REGION1, carnumber, cardirection, NORTH);
		
            P(SE_SEM);
            message(REGION2, carnumber, cardirection, NORTH);
            V(SW_SEM);
		
            P(NE_SEM);
            message(REGION3, carnumber, cardirection, NORTH);
            V(SE_SEM);
		
            message(LEAVING, carnumber, cardirection, NORTH);
            V(NE_SEM);
            break;

	    case EAST:
            message(APPROACHING, carnumber, cardirection, SOUTH);
            P(NE_SEM);
            message(REGION1, carnumber, cardirection, SOUTH);
		
            P(NW_SEM);
            message(REGION2, carnumber, cardirection, SOUTH);
            V(NE_SEM);
		
            P(SW_SEM);
            message(REGION3, carnumber, cardirection, SOUTH);
            V(NW_SEM);
		
            message(LEAVING, carnumber, cardirection, SOUTH);
            V(SW_SEM);
            break;
        default:
            assert(0 && "UNKNOWN ERROR");
            break;
	}

	V(prevent_deadlock);
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
	switch (cardirection){
	    case NORTH:
            message(APPROACHING, carnumber, cardirection, WEST);
            P(NW_SEM);
            message(REGION1, carnumber, cardirection, WEST);
            message(LEAVING, carnumber, cardirection, WEST);
            V(NW_SEM);
            break;
	    case SOUTH:
            message(APPROACHING, carnumber, cardirection, EAST);
            P(SE_SEM);
            message(REGION1, carnumber, cardirection, EAST);
            message(LEAVING, carnumber, cardirection, EAST);
            V(SE_SEM);
            break;
	    case EAST:
            message(APPROACHING, carnumber, cardirection, NORTH);
            P(NE_SEM);
            message(REGION1, carnumber, cardirection, NORTH);
            message(LEAVING, carnumber, cardirection, NORTH);
            V(NE_SEM);
            break;
	    case WEST:
            message(APPROACHING, carnumber, cardirection, SOUTH);
            P(SW_SEM);
            message(REGION1, carnumber, cardirection, SOUTH);
            message(LEAVING, carnumber, cardirection, SOUTH);
            V(SW_SEM);
            break;
        default:
            assert(0 && "UNKNOWN ERROR");
            break;
	}
	V(prevent_deadlock);

}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;
		int turnDirection;
        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;

        /*
         * cardirection is set randomly.
         */

    cardirection = random() % 4;
	turnDirection = random() % 3;
    volatile int count_left=0,count_right=0,count_straight=0;
    volatile int count_north=0,count_south=0,count_west=0,count_east=0;
    switch (cardirection) {
        case NORTH:
            count_north ++;
            break;
        case SOUTH:
            count_south ++;
            break;
        case WEST:
            count_west ++;
            break;
        case EAST:
            count_east ++;
            break;
        default:
            break;
    }
    switch (turnDirection) {
        case GO_STRAIGHT:
            count_straight ++;
            break;
        case TURN_RIGHT:
            count_right ++;
            break;
        case TURN_LEFT:
            count_left ++;
            break;
        default:
            break;
    }
    

	P(prevent_deadlock);//lock if 3 cars are inside
	
	switch(turnDirection){
	case GO_STRAIGHT:
		gostraight(cardirection, carnumber);
		break;
	case TURN_LEFT:
		turnleft(cardirection,carnumber);
		break;
	case TURN_RIGHT:
		turnright(cardirection,carnumber);
		break;
	}
    
    //very last line to release
    V(thread_wait);
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
    
    
    thread_wait = sem_create("thread_wait",0);
    if (thread_wait == NULL) {
        panic("stoplight.c: sem_create thread_wait fail\n");
    }
    prevent_deadlock = sem_create("prevent_deadlock",3);
    if (prevent_deadlock == NULL) {
        panic("stoplight.c: sem_create prevent_deadlock fail\n");
    }
    //NW_SEM semaphore lock
    NW_SEM = sem_create("NW_SEM",1);
    if (NW_SEM == NULL) {
        panic("stoplight.c: sem_create NW_SEM fail\n");
    }
    //NE_SEM semaphore lock
    NE_SEM = sem_create("NE_SEM",1);
    if (NE_SEM == NULL) {
        panic("stoplight.c: sem_create NE_SEM fail\n");
    }
    //SW_SEM semaphore lock
    SW_SEM = sem_create("SW_SEM",1);
    if (SW_SEM == NULL) {
        panic("stoplight.c: sem_create SW_SEM fail\n");
    }
    //SE_SEM semaphore lock
    SE_SEM = sem_create("SE_SEM",1);
    if (SE_SEM == NULL) {
        panic("stoplight.c: sem_create SE_SEM fail\n");
    }

        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }
    int i ;
    for (i = 0; i < NCARS; i++) {
        P(thread_wait);
    }
    
    sem_destroy(NE_SEM);
    sem_destroy(NW_SEM);
    sem_destroy(SW_SEM);
    sem_destroy(SE_SEM);
    sem_destroy(prevent_deadlock);
    sem_destroy(thread_wait);
    return 0;
}
