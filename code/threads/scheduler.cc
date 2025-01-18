// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"
#include "kernel.h"
//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------
int compareP(Thread* x, Thread* y){
    int a = x->getPriority();
    int b = y->getPriority();
    // cout << "compare"<<(b-a)<<"\n";
    if (a==b) return 1;
    else return (b - a);
} 

int compareB(Thread* x, Thread* y){
    int a = x->getBurst();
    int b = y->getBurst();
    // cout << "compare"<<(b-a)<<"\n";
    if (a==b) return 1;
    else return (a - b);
} 
int compareF(Thread* x, Thread* y){

    return (0);
} 

void Wait(Thread *thread){
    thread->setWait(thread->getWait()+100);
}

void Priority_update(Thread *thread){
    // DEBUG(dbgschedule,  "[C] Tick ["<<kernel->stats->totalTicks<<"] thread["<<thread->getID()<<"]  its priority  ["<<thread->getPriority()<<"] " );
    int n;
    if(thread->getPriority()<50) n=3;
    else if(thread->getPriority()<100) n=2;
    else n=1;
    if(thread->getWait() >= 1500){
        thread->setWait(0);
        thread->setPriority(thread->getPriority()+10);
        if(thread->getPriority()>50*(4-n)){
            kernel->scheduler->remove(thread,n);
            kernel->scheduler->ReadyToRun(thread);
        }
        DEBUG(dbgschedule, "[C] Tick ["<<kernel->stats->totalTicks<<"] thread["<<thread->getID()<<"] changes its priority from ["<<thread->getPriority()-10<<"] to ["<<thread->getPriority()<<"]" );
    }
    
}

Scheduler::Scheduler()
{ 
    // L2 = new List<Thread *>; 
    L2 = new SortedList<Thread *>(compareP);
    L1 = new SortedList<Thread *>(compareB);
    // L3 = new SortedList<Thread *>(compareF);
    L3 = new List<Thread *>; 
    // priorityQueue = new List<Thread *>;
    toBeDestroyed = NULL;
} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete L2; 
    delete L1;
    delete L3;
    // delete priorityQueue;
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------
void 
Scheduler::remove(Thread* thread, int n)
{
    if(n==1) L1->Remove(thread);
    else if(n==2) L2->Remove(thread);
    else L3->Remove(thread);
}
void 
Scheduler::PlusWait()
{
    L1->Apply(Wait);
    L2->Apply(Wait);
    L3->Apply(Wait);
}

void 
Scheduler::UpdatePriority()
{
    L1->Apply(Priority_update);
    L2->Apply(Priority_update);
    L3->Apply(Priority_update);
}


void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    // DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
	//cout << "Putting thread on ready list: " << thread->getName() << endl ;    
    thread->setStatus(READY);
    if (thread->getPriority()<50){
        L3->Append(thread);
        // L3->Insert(thread);
        // cout<< "sg\n";
        DEBUG(dbgschedule, "[A] Tick ["<<kernel->stats->totalTicks<<"] thread["<<thread->getID()<<"] is inserted into queue L3" );
    } else if(thread->getPriority()<100){
        L2->Insert(thread);
        DEBUG(dbgschedule, "[A] Tick ["<<kernel->stats->totalTicks<<"] thread["<<thread->getID()<<"] is inserted into queue L2" );
    } else {
        L1->Insert(thread);
        DEBUG(dbgschedule, "[A] Tick ["<<kernel->stats->totalTicks<<"] thread["<<thread->getID()<<"] is inserted into queue L1" );
        if (thread->getBurst() < kernel->currentThread->getBurst()){
            kernel->L1_flag = 1;
        } else kernel->L1_flag = 0;
    }
    //thread->setStatus(READY);
    // if(thread->getID()>=1)
    //     DEBUG(dbgschedule, "[A] Tick ["<<kernel->stats->totalTicks<<"] thread["<<thread->getID()<<"] is inserted into queue" );
    
    // priorityQueue->Append(thread);
}

//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{

    ASSERT(kernel->interrupt->getLevel() == IntOff);
    if (L1->IsEmpty() && L2->IsEmpty() && L3->IsEmpty())
        return NULL;
    else if(!L1->IsEmpty()){

        Thread* gogo = L1->RemoveFront();
        DEBUG(dbgschedule, "[B] Tick ["<<kernel->stats->totalTicks<<"] thread["<<gogo->getID()<<"] is removed from queue L1" );
        // cout<<"thread"<<gogo->getID()<<" "<<gogo->getPriority()<<"removed from queue\n";
    	return gogo;         
    } else if(!L2->IsEmpty()){

        Thread* gogo = L2->RemoveFront();
        DEBUG(dbgschedule, "[B] Tick ["<<kernel->stats->totalTicks<<"] thread["<<gogo->getID()<<"] is removed from queue L2" );
        // cout<<"thread "<<gogo->getID()<<" "<<gogo->getPriority()<<"removed from L2\n";
    	return gogo;          
    } else if(!L3->IsEmpty()){
        Thread* gogo = L3->RemoveFront();
        DEBUG(dbgschedule, "[B] Tick ["<<kernel->stats->totalTicks<<"] thread["<<gogo->getID()<<"] is removed from queue L3" );
        // cout<<"thread"<<gogo->getID()<<" "<<gogo->getPriority()<<"removed from queue\n";
    	return gogo;          
    } else{
        DEBUG(dbgschedule,"GG no thread in queue");
    }
    // if (L2->IsEmpty()) {
	// 	return NULL;
    // } else {
    //     Thread* gogo = L2->RemoveFront();
    //     DEBUG(dbgschedule, "[B] Tick ["<<kernel->stats->totalTicks<<"] thread["<<gogo->getID()<<"] is removed from queue" );
    //     // cout<<"thread"<<gogo->getID()<<" "<<gogo->getPriority()<<"removed from queue\n";
    // 	return gogo; 
    // }
    // if (priorityQueue->IsEmpty()) {
	// 	return NULL;
    // } else {
    // 	return priorityQueue->RemoveFront();
    // }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{

    if(kernel->alarm_flag){
        DEBUG(dbgschedule,"\nYield to Thread["<<nextThread->getID()<<"]\n");
    }
    Thread *oldThread = kernel->currentThread;
    nextThread->setWait(0);
    oldThread->setPriority(oldThread->getOPriority());
    int during = kernel->stats->totalTicks - current;
    if(!kernel->alarm_flag ){
        if(during>1){
            double oldBurst = oldThread->getBurst();
            oldThread->setBurst(during+oldThread->getLWait());
            DEBUG(dbgschedule, "[D] Tick ["<<kernel->stats->totalTicks<<"] thread["<<oldThread->getID()<<"] update approximate burst time, from ["<<oldBurst<<"], add ["<<during<<"], to ["<<oldThread->getBurst()<<"]" );
        }
        DEBUG(dbgschedule, "[E] Tick ["<<kernel->stats->totalTicks<<"] thread["<<nextThread->getID()<<"] is now selected for execution, thread["<<oldThread->getID()<<"] is replace and it has executed "<<oldThread->getLWait()<<" ticks" );
        oldThread->setLWait(0);
    } else {
        kernel->alarm_flag = 0;
        oldThread->setLWait(during+oldThread->getLWait());
        DEBUG(dbgschedule, "[E] Tick ["<<kernel->stats->totalTicks<<"] thread["<<nextThread->getID()<<"] is now selected for execution, thread["<<oldThread->getID()<<"] is replace and it has executed "<<oldThread->getLWait()<<" ticks" );
    
    }
    current = kernel->stats->totalTicks;
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	 toBeDestroyed = oldThread;
    }
    
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	oldThread->space->SaveState();
    }
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    SWITCH(oldThread, nextThread);

    // we're back, running oldThread
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
    kernel->RR = 1;
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    L2->Apply(ThreadPrint);
    cout <<"\n";
}
