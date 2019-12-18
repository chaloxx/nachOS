/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"
#include "synch.hh"

/// Loop 10 times, yielding the CPU to another ready thread each iteration.
///
/// * `name` points to a string with a thread name, just for debugging
///   purposes.




#ifdef SEMAPHORE_TEST
Semaphore *s = new Semaphore(NULL,3);
#endif

void SimpleThread(void *name_){
    
    char *name = (char *) name_;
    #ifdef SEMAPHORE_TEST
    s-> P();
    DEBUG('d',"Soy %s y hice P \n",name);
    #endif
    // Reinterpret arg `name` as a string.


    // If the lines dealing with interrupts are commented, the code will
    // behave incorrectly, because printf execution may cause race
    // conditions.
    for (unsigned num = 0; num < 10; num++) {
        printf("*** Thread `%s` is running: iteration %u\n", name, num);
        currentThread->Yield();
    }
    printf("!!! Thread `%s` has finished\n", name);
    #ifdef SEMAPHORE_TEST
    DEBUG('d',"Soy %s y hice V \n",name);
    s-> V();
    #endif
}
/// Set up a ping-pong between several threads.
///
/// Do it by launching ten threads which call `SimpleThread`, and finally
/// calling `SimpleThread` ourselves.


void ThreadTest(){   
    int ctos = 5;
    char* names[ctos];
    Thread *threads[ctos];
    for (int i = 0; i < ctos; i++){
      names[i] = new char[8];
      sprintf(names[i],"hilo::%d",i);
      threads[i] = new Thread(names[i]);
      threads[i]->Fork(SimpleThread,(void *) names[i]);
    }
    
}




