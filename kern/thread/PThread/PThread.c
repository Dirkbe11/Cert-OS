#include <lib/x86.h>
#include <lib/thread.h>
#include <lib/spinlock.h>
#include <lib/debug.h>
#include <dev/lapic.h>
#include <pcpu/PCPUIntro/export.h>
#include <kern/thread/PTCBIntro/export.h>

#include "import.h"

static spinlock_t sched_lk;

unsigned int sched_ticks[NUM_CPUS];

void thread_init(unsigned int mbi_addr)
{
  unsigned int i;
  for(i = 0; i < NUM_CPUS; i++) {
    sched_ticks[i] = 0;
  }

  spinlock_init(&sched_lk);
  tqueue_init(mbi_addr);
  set_curid(0);
  tcb_set_state(0, TSTATE_RUN);
}

/**
 * Allocates new child thread context, set the state of the new child thread
 * as ready, and pushes it to the ready queue.
 * It returns the child thread id.
 */
unsigned int thread_spawn(void *entry, unsigned int id, unsigned int quota)
{
  unsigned int pid;

  spinlock_acquire(&sched_lk);

  pid = kctx_new(entry, id, quota);
  tcb_set_state(pid, TSTATE_READY);
  tcb_set_cpu(pid, get_pcpu_idx());
  tqueue_enqueue(NUM_IDS + get_pcpu_idx(), pid);
	
  spinlock_release(&sched_lk);
  
  return pid;
}

/**
 * Yield to the next thread in the ready queue.
 * You should set the currently running thread state as ready,
 * and push it back to the ready queue.
 * And set the state of the poped thread as running, set the
 * current thread id, then switches to the new kernel context.
 * Hint: if you are the only thread that is ready to run,
 * do you need to switch to yourself?
 */
void thread_yield(void)
{
  unsigned int old_cur_pid;
  unsigned int new_cur_pid;

  spinlock_acquire(&sched_lk);

  old_cur_pid = get_curid();
  tcb_set_state(old_cur_pid, TSTATE_READY);
  tqueue_enqueue(NUM_IDS + get_pcpu_idx(), old_cur_pid);

  new_cur_pid = tqueue_dequeue(NUM_IDS + get_pcpu_idx());
  tcb_set_state(new_cur_pid, TSTATE_RUN);
  set_curid(new_cur_pid);

  if (old_cur_pid != new_cur_pid){
    spinlock_release(&sched_lk);
    kctx_switch(old_cur_pid, new_cur_pid);
  }
  else {
    spinlock_release(&sched_lk);
  }
}

void sched_update(void)
{
  spinlock_acquire(&sched_lk);
  sched_ticks[get_pcpu_idx()] += (1000 / LAPIC_TIMER_INTR_FREQ);
  if (sched_ticks[get_pcpu_idx()] > SCHED_SLICE) {
    sched_ticks[get_pcpu_idx()] = 0;
    spinlock_release(&sched_lk);
    thread_yield();
  }
  else{
    spinlock_release(&sched_lk);
  }
}


/**
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 */
void thread_sleep (void *chan, spinlock_t *lk)
{
  //TODO: your local variables here.
  unsigned int cur_id;
  cur_id = get_curid();
  unsigned int new_id;
  
  if (lk == 0)
    KERN_PANIC("sleep without lock");

  spinlock_acquire(&sched_lk);
  //  tcb_set_state(cur_id, TSTATE_READY);

  // TODO
  // Must acquire sched_lk in order to
  // change the current thread's state and then switch.
  // Once we hold sched_lk, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with sched_lk locked),
  // so it's okay to release lock.

  spinlock_release(lk);  // release lk

  tcb_set_state(cur_id, TSTATE_SLEEP);  //set state to sleep
  tcb_set_chan(cur_id, chan); //set the channel that the sleeping thread is waitng for
  //tqueue_enqueue(NUM_IDS, cur_id);  //add sleeping thread to queue

  new_id = tqueue_dequeue(NUM_IDS); //get the next queue
  tcb_set_state(new_id, TSTATE_RUN);  //set thread state to run
  set_curid(new_id);  //set the current ID
  spinlock_release(&sched_lk);  //release schedlock

  kctx_switch(cur_id, new_id);  //context swiiitch
  spinlock_acquire(&sched_lk); //get shedlock back

  tcb_set_chan(cur_id, 0);    //set channel thisthread is depending on to 0, none

  spinlock_release(&sched_lk);  //rleease sched lock
  spinlock_acquire(lk); //get lk

  // TODO: Go to sleep.

  // TODO: Context switch.
  
  // TODO: Tidy up.

  // TODO: Reacquire original lock.

}

/**
 * Wake up all processes sleeping on chan.
 */
void thread_wakeup (void *chan)
{

  //acquire lock
  spinlock_acquire(&sched_lk);

  //skip proc 0->kern
  for (unsigned int i = 1; i < NUM_IDS; ++i)
  {
    if (tcb_get_chan(i) == chan && tcb_get_state(i) == TSTATE_SLEEP)
    {
      tcb_set_state(i, TSTATE_READY);
      tcb_set_chan(i, 0);
      tqueue_enqueue(NUM_IDS, i);

    }
    
  }

  spinlock_release(&sched_lk);

  //TODO
}
