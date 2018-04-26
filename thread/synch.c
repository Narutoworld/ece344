/*
 * Synchronization primitives.
 * See synch.h for specifications of the functions.
 */

#include <types.h>
#include <lib.h>
#include <synch.h>
#include <thread.h>
#include <curthread.h>
#include <machine/spl.h>

extern struct thread *curthread;
////////////////////////////////////////////////////////////
//
// Semaphore.

struct semaphore *
sem_create(const char *namearg, int initial_count)
{
	struct semaphore *sem;

	assert(initial_count >= 0);

	sem = kmalloc(sizeof(struct semaphore));
	if (sem == NULL) {
		return NULL;
	}

	sem->name = kstrdup(namearg);
	if (sem->name == NULL) {
		kfree(sem);
		return NULL;
	}

	sem->count = initial_count;
	return sem;
}

void
sem_destroy(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	spl = splhigh();
	assert(thread_hassleepers(sem)==0);
	splx(spl);

	/*
	 * Note: while someone could theoretically start sleeping on
	 * the semaphore after the above test but before we free it,
	 * if they're going to do that, they can just as easily wait
	 * a bit and start sleeping on the semaphore after it's been
	 * freed. Consequently, there's not a whole lot of point in 
	 * including the kfrees in the splhigh block, so we don't.
	 */

	kfree(sem->name);
	kfree(sem);
}

void 
P(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);

	/*
	 * May not block in an interrupt handler.
	 *
	 * For robustness, always check, even if we can actually
	 * complete the P without blocking.
	 */
	assert(in_interrupt==0);

	spl = splhigh();
	while (sem->count==0) {
		thread_sleep(sem);
	}
	assert(sem->count>0);
	sem->count--;
	splx(spl);
}

void
V(struct semaphore *sem)
{
	int spl;
	assert(sem != NULL);
	spl = splhigh();
	sem->count++;
	assert(sem->count>0);
	thread_wakeup(sem);
	splx(spl);
}

////////////////////////////////////////////////////////////
//
// Lock.

struct lock *
lock_create(const char *name)
{
	struct lock *lock;

	lock = kmalloc(sizeof(struct lock));
	if (lock == NULL) {
		return NULL;
	}

	lock->name = kstrdup(name);
	if (lock->name == NULL) {
		kfree(lock);
		return NULL;
	}
	
	// add stuff here as needed
	lock->held = 0;
	lock->owner = NULL;
	
	return lock;
}

void
lock_destroy(struct lock *lock)
{
	assert(lock != NULL);

	// add stuff here as needed
	int spl;
	spl = splhigh();
	assert(lock->held == 0);
	assert(lock->owner == NULL);
	splx(spl);
	
	kfree(lock->name);
	kfree(lock);
	lock = NULL;
}

void
lock_acquire(struct lock *lock)
{
	// Write this
	int spl;
	assert(lock!= NULL);

	assert(in_interrupt==0);
	spl = splhigh();		
	while(lock->held != 0){
		thread_sleep(lock);
	}
	assert(lock->held == 0);
	lock->held = 1;
	lock->owner = curthread;

	splx(spl);

//	(void)lock;  // suppress warning until code gets written
}

void
lock_release(struct lock *lock)
{
	// Write this
	int spl;
	assert(lock!=NULL);
	spl = splhigh();
	thread_wakeup(lock);
	lock->held = 0;
	lock->owner = NULL;
	assert(lock->held==0);
	assert(lock->owner == NULL);
	splx(spl);
	//(void)lock;  // suppress warning until code gets written
}

int
lock_do_i_hold(struct lock *lock)
{
	// Write this
	int spl;
	assert(lock!=NULL);
	spl = splhigh();
	
	if (lock->owner == curthread){
		splx(spl);
		return 1;
	}

	//(void)lock;  // suppress warning until code gets written
	else{
		splx(spl);
		return 0;
	}
}

////////////////////////////////////////////////////////////
//
// CV


struct cv *
cv_create(const char *name)
{
	struct cv *cv;

	cv = kmalloc(sizeof(struct cv));
	if (cv == NULL) {
		return NULL;
	}

	cv->name = kstrdup(name);
	if (cv->name==NULL) {
		kfree(cv);
		return NULL;
	}
	
	// add stuff here as needed
	cv->count = 0;	
	
	return cv;
}

void
cv_destroy(struct cv *cv)
{
	int spl = splhigh();
	assert(cv != NULL);

	// add stuff here as needed
	//before destroy it, all thread waiting on this should be waken up
	assert(cv->count==0);
	
	kfree(cv->name);
	kfree(cv);
	//reset cv
	cv = NULL;
	splx(spl);
}

void
cv_wait(struct cv *cv, struct lock *lock)
{
	// Write this
	assert(cv!=NULL);
	assert(lock!=NULL);
	//the current thread must hold the lock passed in
	assert(lock_do_i_hold(lock));
	
	int spl;
	
	lock_release(lock);	

	spl = splhigh();	
	cv->count++;
	//thread go to sleep
	thread_sleep(cv);
	splx(spl);
	
	lock_acquire(lock);

	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
}

void
cv_signal(struct cv *cv, struct lock *lock)
{
	// Write this
	int spl;
	assert(cv!=NULL);
	assert(lock!=NULL);
	//the current thread must hold the lock passed in
	assert(lock_do_i_hold(lock));

	spl = splhigh();
	// copy from thread_wakeup
	if (thread_hassleepers(cv)){
		thread_wakeupone(cv);
	}
	else{
		kprintf("error, there is no avaliable thread to wake up");
		cv->count = 0;
	}	
	cv->count--;
	splx(spl);
	
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
}

void
cv_broadcast(struct cv *cv, struct lock *lock)
{
	// Write this
	int spl;
	assert(cv!=NULL);
	assert(lock!=NULL);
	//the current thread must hold the lock passed in
	assert(lock_do_i_hold(lock));
	
	spl = splhigh();

	thread_wakeup(cv);
	cv->count = 0;

	splx(spl);
	//(void)cv;    // suppress warning until code gets written
	//(void)lock;  // suppress warning until code gets written
}
