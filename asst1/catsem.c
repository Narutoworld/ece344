/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
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
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

// numbers of total iteration
#define TOTAL_ITERATION 4

//define what is available, and what is not available
#define AVAILABLE 0
#define NOT_AVAILABLE 1

static struct semaphore* thread_wait = NULL;
// represent there are only two dishes available
static struct semaphore* dishes = NULL;
// use to protest other shared variable such as num_cat_eating
static struct semaphore* protect = NULL;

//Below they are the shared variables by the total 8 thread
static int num_cat_eating = 0;
static int num_mouse_eating = 0;
static int dish_one_available = 0;
static int dish_two_available = 0;
/*
 * 
 * Function Definitions
 * 
 */
/* useful and interesting
typedef enum 
{
    dish1_avail,
    dish2_avail,
    not_avail    
}dish_availability;

static dish_availability dish_available[NFOODBOWLS];
*/

/* who should be "cat" or "mouse" */
static void
sem_eat(const char *who, int num, int bowl, int iteration)
{
        kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
        clocksleep(1);
        kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num, 
                bowl, iteration);
}

/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
    // the iteration is NOT a shared resource between differnet thread;
    // eash iteration correspond one spacific thread
    // not need to protect it
    int iteration;
    for (iteration = 0; iteration < TOTAL_ITERATION; iteration++) {
        //Restrict by the only two dishes
        P(dishes);
        P(protect);
        if (num_mouse_eating > 0) {
            //no cat should enter the function
            V(protect);
            V(dishes);
            iteration --;
            continue;
        }
        
        //The number of mouse is zero
        if (dish_one_available == AVAILABLE) {
            dish_one_available = NOT_AVAILABLE;
            num_cat_eating ++;
            V(protect);
            sem_eat("cat",catnumber,1,iteration);
            P(protect);
            dish_one_available = AVAILABLE;
            num_cat_eating--;
            V(protect);
        }
        else if(dish_two_available == AVAILABLE){
            dish_two_available = NOT_AVAILABLE;
            num_cat_eating ++;
            V(protect);
            sem_eat("cat",catnumber,2,iteration);
            P(protect);
            dish_two_available = AVAILABLE;
            num_cat_eating--;
            V(protect);
        }
        else {
            V(protect);
            iteration --;
        }
        
        V(dishes);
    }
    (void) unusedpointer;
    V(thread_wait);
}
        

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer, 
         unsigned long mousenumber)
{
    //similar to catsem
    int iteration = 0;
    for (iteration = 0; iteration < TOTAL_ITERATION; iteration++) {
        //Restrict by the only two dishes
        P(dishes);
		P(protect);
        if (num_cat_eating > 0) {
            //no mouse should enter the function
			V(protect);
            V(dishes);
            iteration --;
            continue;
        }
        
        //The number of cat is zero
        if (dish_one_available == AVAILABLE) {
            dish_one_available = NOT_AVAILABLE;
            num_mouse_eating ++;
            V(protect);
            sem_eat("mouse",mousenumber,1,iteration);
            P(protect);
            dish_one_available = AVAILABLE;
            num_mouse_eating--;
            V(protect);
        }
        else if(dish_two_available == AVAILABLE){
            dish_two_available = NOT_AVAILABLE;
            num_mouse_eating ++;
            V(protect);
            sem_eat("mouse",mousenumber,2,iteration);
            P(protect);
            dish_two_available = AVAILABLE;
            num_mouse_eating--;
            V(protect);
        }
        else {
            V(protect);
            iteration --;
        }
        
        V(dishes);
    }

    (void) unusedpointer;
    V(thread_wait);
}


/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
            char ** args)
{
        int index, error;
   
        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;
   
        /*
         * Start NCATS catsem() threads.
         */
    
        thread_wait = sem_create("thread_wait",0);
        if (thread_wait == NULL) {
            panic("catsem.c: sem_create thread_wait fail\n");
        }
        dishes = sem_create("dishes",2);
        if (dishes == NULL) {
            panic("catsem.c: sem_create dishes fail\n");
        }
        protect = sem_create("protext",1);
        if (protect == NULL) {
             panic("catsem.c: sem_create protect fail\n");
        }
        //initial all the shared variable,before actually create multiple thread
        num_cat_eating = 0;
        num_mouse_eating = 0;
        dish_one_available = AVAILABLE;
        dish_two_available = AVAILABLE;

        for (index = 0; index < NCATS; index++) {
           
                error = thread_fork("catsem Thread", 
                                    NULL, 
                                    index, 
                                    catsem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
                 
                        panic("catsem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
        
        /*
         * Start NMICE mousesem() threads.
         */

        for (index = 0; index < NMICE; index++) {
   
                error = thread_fork("mousesem Thread", 
                                    NULL, 
                                    index, 
                                    mousesem, 
                                    NULL
                                    );
                
                /*
                 * panic() on error.
                 */

                if (error) {
         
                        panic("mousesem: thread_fork failed: %s\n", 
                              strerror(error)
                              );
                }
        }
    
    //this general method are used to keep only one thread are running in a particular time
    //if the thread has not been relased yet, the thread will sleep here
    //After all lock released, we can finally move out of the braces
    int i;
    for (i = 0; i < 8; i++) {
        P(thread_wait);
    }
    
    sem_destroy(thread_wait);
    sem_destroy(dishes);
    sem_destroy(protect);
    
    return 0;
}


/*
 * End of catsem.c
 */
