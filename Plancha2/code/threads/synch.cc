/// Routines for synchronizing threads.
///
/// Three kinds of synchronization routines are defined here: semaphores,
/// locks and condition variables (the implementation of the last two are
/// left to the reader).
///
/// Any implementation of a synchronization routine needs some primitive
/// atomic operation.  We assume Nachos is running on a uniprocessor, and
/// thus atomicity can be provided by turning off interrupts.  While
/// interrupts are disabled, no context switch can occur, and thus the
/// current thread is guaranteed to hold the CPU throughout, until interrupts
/// are reenabled.
///
/// Because some of these routines might be called with interrupts already
/// disabled (`Semaphore::V` for one), instead of turning on interrupts at
/// the end of the atomic operation, we always simply re-set the interrupt
/// state back to its original value (whether that be disabled or enabled).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "synch.hh"
#include "system.hh"


/// Initialize a semaphore, so that it can be used for synchronization.
///
/// * `debugName` is an arbitrary name, useful for debugging.
/// * `initialValue` is the initial value of the semaphore.
Semaphore::Semaphore(const char *debugName, int initialValue)
{
    name  = debugName;
    value = initialValue;
    queue = new List<Thread *>;
}

/// De-allocate semaphore, when no longer needed.
///
/// Assume no one is still waiting on the semaphore!
Semaphore::~Semaphore()
{
    delete queue;
}

const char *
Semaphore::GetName() const
{
    return name;
}

/// Wait until semaphore `value > 0`, then decrement.
///
/// Checking the value and decrementing must be done atomically, so we need
/// to disable interrupts before checking the value.
///
/// Note that `Thread::Sleep` assumes that interrupts are disabled when it is
/// called.

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);
      // Disable interrupts.

    while (value == 0) {  // Semaphore not available.
        queue->Append(currentThread);  // So go to sleep.
        currentThread->Sleep();
    }
    value--;  // Semaphore available, consume its value.

    interrupt->SetLevel(oldLevel);  // Re-enable interrupts.
}

/// Increment semaphore value, waking up a waiter if necessary.
///
/// As with `P`, this operation must be atomic, so we need to disable
/// interrupts.  `Scheduler::ReadyToRun` assumes that threads are disabled
/// when it is called.
void
Semaphore::V()
{
    IntStatus oldLevel = interrupt->SetLevel(INT_OFF);

    Thread *thread = queue->Pop();
    if (thread != nullptr)
        // Make thread ready, consuming the `V` immediately.
        scheduler->ReadyToRun(thread,currentThread-> GetPriority());
    value++;

    interrupt->SetLevel(oldLevel);
}

/// Dummy functions -- so we can compile our later assignments.
///
/// Note -- without a correct implementation of `Condition::Wait`, the test
/// case in the network assignment will not work!

//Locks
//-------------------------------------------------------
Lock::Lock(const char *debugName)
{	
   name = debugName;
   s = new Semaphore(debugName,1);
   lockThread = nullptr;
   oldPriority = -1;
}

Lock::~Lock()
{
  DEBUG('s',"Eliminando lock %s",currentThread->GetName());
  delete s;
}

const char *
Lock::GetName() const
{
    return name;
}

void
Lock::Acquire()
{
   // Comprobar que el hilo actual no ah tomado el lock
  ASSERT(!IsHeldByCurrentThread());
  DEBUG('s',"%s intentando tomar lock con prioridad %d \n",currentThread-> GetName(), currentThread->GetPriority());
  // Comprobar si existe inversión de prioridad
  if(lockThread != nullptr &&  lockThread-> GetPriority() < currentThread -> GetPriority()){
    oldPriority = lockThread -> GetPriority(); // Almacenar vieja prioridad
    lockThread -> SetPriority(currentThread->GetPriority()); // Heredar prioridad
    DEBUG('s',"Cambiando prioridad de %s a %d\n",lockThread->GetName(),lockThread->GetPriority());
  }
  // Tomar lock
  s->P();
  DEBUG('l',"%s tomando lock %s \n",currentThread-> GetName(), name);
  // Almacenar que hilo posee el mutex
  lockThread = currentThread;
}

void
Lock::Release()
{
  DEBUG('l',"Hilo %s liberando lock %s \n",currentThread-> GetName(), name);
  // Comprobar que se posee un mutex
  ASSERT(IsHeldByCurrentThread());
  if(oldPriority != -1){
    lockThread -> SetPriority(oldPriority); // Reestablecer vieja prioridad si hubo herencia
    DEBUG('s',"La prioridad de %s vuelve a %d\n",lockThread->GetName(),oldPriority);
    oldPriority = -1;
  }
  lockThread = nullptr;
  s->V(); // Soltar lock
}

bool
Lock::IsHeldByCurrentThread() const
{
  // Podemos identificar a cada hilo por su dirección de memoria
  return lockThread == currentThread;
}
  
//--------------------------------------------------------------
// Conditions


Condition::Condition(const char *debugName, Lock *conditionLock)
{
  ASSERT(conditionLock != nullptr);
  name = debugName;
  // Asociar condición con lock
  l = conditionLock; 
  // Necesitaremos una cola de semáforos
  queue = new List<Semaphore *>;
}

Condition::~Condition()
{
  delete queue; // Destruir cola
}

const char *
Condition::GetName() const
{
    return name;
}

void imprimir(Semaphore* s){
  printf("%s\n",s->GetName());
}


void
Condition::Wait()
{
  s = new Semaphore(currentThread->GetName(),0); // Creamos un semáforo para poner a dormir al hilo hasta que llega la signal
  queue->Append(s); // Agregamos el semáforo en la cola
  l->Release();// Soltar el mutex hasta que se reciba la signal
  s-> P();// Dormir hasta que llegue la signal
  l->Acquire();//Retomar lock
}

void
Condition::Signal()
{

 if (!queue->IsEmpty()){
   s = queue->Pop(); // Tomar semáforo al comienzo de la lista
   s-> V();//Enviar signal
 }

}


//Función para el broadcast
void callV(Semaphore* s){
  s-> V();
}


void
Condition::Broadcast()
{
  queue-> Apply(callV);// Despertar a todos los threads en la cola
}
